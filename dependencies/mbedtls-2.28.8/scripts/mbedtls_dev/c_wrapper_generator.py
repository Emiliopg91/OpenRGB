"""Generate C wrapper functions."""

# Copyright The Mbed TLS Contributors
# SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later

### WARNING: the code in this file has not been extensively reviewed yet.
### We do not think it is harmful, but it may be below our normal standards
### for robustness and maintainability.

import os
import re
import sys
import typing
from typing import Dict, List, Optional, Tuple

from .c_parsing_helper import ArgumentInfo, FunctionInfo
from . import typing_util


def c_declare(prefix: str, name: str, suffix: str) -> str:
    """Format a declaration of name with the given type prefix and suffix."""
    if not prefix.endswith("*"):
        prefix += " "
    return prefix + name + suffix


WrapperInfo = typing.NamedTuple(
    "WrapperInfo",
    [
        ("argument_names", List[str]),
        ("guard", Optional[str]),
        ("wrapper_name", str),
    ],
)


class Base:
    """Generate a C source file containing wrapper functions."""

    # This class is designed to have many methods potentially overloaded.
    # Tell pylint not to complain about methods that have unused arguments:
    # child classes are likely to override those methods and need the
    # arguments in question.
    # pylint: disable=no-self-use,unused-argument

    # Prefix prepended to the function's name to form the wrapper name.
    _WRAPPER_NAME_PREFIX = ""
    # Suffix appended to the function's name to form the wrapper name.
    _WRAPPER_NAME_SUFFIX = "_wrap"

    # Functions with one of these qualifiers are skipped.
    _SKIP_FUNCTION_WITH_QUALIFIERS = frozenset(["inline", "static"])

    def __init__(self):
        """Construct a wrapper generator object."""
        self.program_name = os.path.basename(sys.argv[0])
        # To be populated in a derived class
        self.functions = {}  # type: Dict[str, FunctionInfo]
        # Preprocessor symbol used as a guard against multiple inclusion in the
        # header. Must be set before writing output to a header.
        # Not used when writing .c output.
        self.header_guard = None  # type: Optional[str]

    def _write_prologue(self, out: typing_util.Writable, header: bool) -> None:
        """Write the prologue of a C file.

        This includes a description comment and some include directives.
        """
        out.write(
            """/* Automatically generated by {}, do not edit! */

/* Copyright The Mbed TLS Contributors
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */
""".format(
                self.program_name
            )
        )
        if header:
            out.write(
                """
#ifndef {guard}
#define {guard}

#ifdef __cplusplus
extern "C" {{
#endif
""".format(
                    guard=self.header_guard
                )
            )
        out.write(
            """
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
"""
        )

    def _write_epilogue(self, out: typing_util.Writable, header: bool) -> None:
        """Write the epilogue of a C file."""
        if header:
            out.write(
                """
#ifdef __cplusplus
}}
#endif

#endif /* {guard} */
""".format(
                    guard=self.header_guard
                )
            )
        out.write(
            """
/* End of automatically generated file. */
"""
        )

    def _wrapper_function_name(self, original_name: str) -> str:
        """The name of the wrapper function.

        By default, this adds a suffix.
        """
        return self._WRAPPER_NAME_PREFIX + original_name + self._WRAPPER_NAME_SUFFIX

    def _wrapper_declaration_start(self, function: FunctionInfo, wrapper_name: str) -> str:
        """The beginning of the wrapper function declaration.

        This ends just before the opening parenthesis of the argument list.

        This is a string containing at least the return type and the
        function name. It may start with additional qualifiers or attributes
        such as `static`, `__attribute__((...))`, etc.
        """
        return c_declare(function.return_type, wrapper_name, "")

    def _argument_name(self, function_name: str, num: int, arg: ArgumentInfo) -> str:
        """Name to use for the given argument in the wrapper function.

        Argument numbers count from 0.
        """
        name = "arg" + str(num)
        if arg.name:
            name += "_" + arg.name
        return name

    def _wrapper_declaration_argument(self, function_name: str, num: int, name: str, arg: ArgumentInfo) -> str:
        """One argument definition in the wrapper function declaration.

        Argument numbers count from 0.
        """
        return c_declare(arg.type, name, arg.suffix)

    def _underlying_function_name(self, function: FunctionInfo) -> str:
        """The name of the underlying function.

        By default, this is the name of the wrapped function.
        """
        return function.name

    def _return_variable_name(self, function: FunctionInfo) -> str:
        """The name of the variable that will contain the return value."""
        return "retval"

    def _write_function_call(
        self, out: typing_util.Writable, function: FunctionInfo, argument_names: List[str]
    ) -> None:
        """Write the call to the underlying function."""
        # Note that the function name is in parentheses, to avoid calling
        # a function-like macro with the same name, since in typical usage
        # there is a function-like macro with the same name which is the
        # wrapper.
        call = "({})({})".format(self._underlying_function_name(function), ", ".join(argument_names))
        if function.returns_void():
            out.write("    {};\n".format(call))
        else:
            ret_name = self._return_variable_name(function)
            ret_decl = c_declare(function.return_type, ret_name, "")
            out.write("    {} = {};\n".format(ret_decl, call))

    def _write_function_return(self, out: typing_util.Writable, function: FunctionInfo, if_void: bool = False) -> None:
        """Write a return statement.

        If the function returns void, only write a statement if if_void is true.
        """
        if function.returns_void():
            if if_void:
                out.write("    return;\n")
        else:
            ret_name = self._return_variable_name(function)
            out.write("    return {};\n".format(ret_name))

    def _write_function_body(
        self, out: typing_util.Writable, function: FunctionInfo, argument_names: List[str]
    ) -> None:
        """Write the body of the wrapper code for the specified function."""
        self._write_function_call(out, function, argument_names)
        self._write_function_return(out, function)

    def _skip_function(self, function: FunctionInfo) -> bool:
        """Whether to skip this function.

        By default, static or inline functions are skipped.
        """
        if not self._SKIP_FUNCTION_WITH_QUALIFIERS.isdisjoint(function.qualifiers):
            return True
        return False

    _FUNCTION_GUARDS = {}  # type: Dict[str, str]

    def _function_guard(self, function: FunctionInfo) -> Optional[str]:
        """A preprocessor condition for this function.

        The wrapper will be guarded with `#if` on this condition, if not None.
        """
        return self._FUNCTION_GUARDS.get(function.name)

    def _wrapper_info(self, function: FunctionInfo) -> Optional[WrapperInfo]:
        """Information about the wrapper for one function.

        Return None if the function should be skipped.
        """
        if self._skip_function(function):
            return None
        argument_names = [self._argument_name(function.name, num, arg) for num, arg in enumerate(function.arguments)]
        return WrapperInfo(
            argument_names=argument_names,
            guard=self._function_guard(function),
            wrapper_name=self._wrapper_function_name(function.name),
        )

    def _write_function_prototype(
        self, out: typing_util.Writable, function: FunctionInfo, wrapper: WrapperInfo, header: bool
    ) -> None:
        """Write the prototype of a wrapper function.

        If header is true, write a function declaration, with a semicolon at
        the end. Otherwise just write the prototype, intended to be followed
        by the function's body.
        """
        declaration_start = self._wrapper_declaration_start(function, wrapper.wrapper_name)
        arg_indent = "    "
        terminator = ";\n" if header else "\n"
        if function.arguments:
            out.write(declaration_start + "(\n")
            for num in range(len(function.arguments)):
                arg_def = self._wrapper_declaration_argument(
                    function.name, num, wrapper.argument_names[num], function.arguments[num]
                )
                arg_terminator = ")" + terminator if num == len(function.arguments) - 1 else ",\n"
                out.write(arg_indent + arg_def + arg_terminator)
        else:
            out.write(declaration_start + "(void)" + terminator)

    def _write_c_function(self, out: typing_util.Writable, function: FunctionInfo) -> None:
        """Write wrapper code for one function.

        Do nothing if the function is skipped.
        """
        wrapper = self._wrapper_info(function)
        if wrapper is None:
            return
        out.write(
            """
/* Wrapper for {} */
""".format(
                function.name
            )
        )
        if wrapper.guard is not None:
            out.write("#if {}\n".format(wrapper.guard))
        self._write_function_prototype(out, function, wrapper, False)
        out.write("{\n")
        self._write_function_body(out, function, wrapper.argument_names)
        out.write("}\n")
        if wrapper.guard is not None:
            out.write("#endif /* {} */\n".format(wrapper.guard))

    def _write_h_function_declaration(
        self, out: typing_util.Writable, function: FunctionInfo, wrapper: WrapperInfo
    ) -> None:
        """Write the declaration of one wrapper function."""
        self._write_function_prototype(out, function, wrapper, True)

    def _write_h_macro_definition(
        self, out: typing_util.Writable, function: FunctionInfo, wrapper: WrapperInfo
    ) -> None:
        """Write the macro definition for one wrapper."""
        arg_list = ", ".join(wrapper.argument_names)
        out.write(
            "#define {function_name}({args}) \\\n    {wrapper_name}({args})\n".format(
                function_name=function.name, wrapper_name=wrapper.wrapper_name, args=arg_list
            )
        )

    def _write_h_function(self, out: typing_util.Writable, function: FunctionInfo) -> None:
        """Write the complete header content for one wrapper.

        This is the declaration of the wrapper function, and the
        definition of a function-like macro that calls the wrapper function.

        Do nothing if the function is skipped.
        """
        wrapper = self._wrapper_info(function)
        if wrapper is None:
            return
        out.write("\n")
        if wrapper.guard is not None:
            out.write("#if {}\n".format(wrapper.guard))
        self._write_h_function_declaration(out, function, wrapper)
        self._write_h_macro_definition(out, function, wrapper)
        if wrapper.guard is not None:
            out.write("#endif /* {} */\n".format(wrapper.guard))

    def write_c_file(self, filename: str) -> None:
        """Output a whole C file containing function wrapper definitions."""
        with open(filename, "w", encoding="utf-8") as out:
            self._write_prologue(out, False)
            for name in sorted(self.functions):
                self._write_c_function(out, self.functions[name])
            self._write_epilogue(out, False)

    def _header_guard_from_file_name(self, filename: str) -> str:
        """Preprocessor symbol used as a guard against multiple inclusion."""
        # Heuristic to strip irrelevant leading directories
        filename = re.sub(r".*include[\\/]", r"", filename)
        return re.sub(r"[^0-9A-Za-z]", r"_", filename, re.A).upper()

    def write_h_file(self, filename: str) -> None:
        """Output a header file with function wrapper declarations and macro definitions."""
        self.header_guard = self._header_guard_from_file_name(filename)
        with open(filename, "w", encoding="utf-8") as out:
            self._write_prologue(out, True)
            for name in sorted(self.functions):
                self._write_h_function(out, self.functions[name])
            self._write_epilogue(out, True)


class UnknownTypeForPrintf(Exception):
    """Exception raised when attempting to generate code that logs a value of an unknown type."""

    def __init__(self, typ: str) -> None:
        super().__init__("Unknown type for printf format generation: " + typ)


class Logging(Base):
    """Generate wrapper functions that log the inputs and outputs."""

    def __init__(self) -> None:
        """Construct a wrapper generator including logging of inputs and outputs.

        Log to stdout by default. Call `set_stream` to change this.
        """
        super().__init__()
        self.stream = "stdout"

    def set_stream(self, stream: str) -> None:
        """Set the stdio stream to log to.

        Call this method before calling `write_c_output` or `write_h_output`.
        """
        self.stream = stream

    def _write_prologue(self, out: typing_util.Writable, header: bool) -> None:
        super()._write_prologue(out, header)
        if not header:
            out.write(
                """
#if defined(MBEDTLS_FS_IO) && defined(MBEDTLS_TEST_HOOKS)
#include <stdio.h>
#include <inttypes.h>
#include <mbedtls/debug.h> // for MBEDTLS_PRINTF_SIZET
#include <mbedtls/platform.h> // for mbedtls_fprintf
#endif /* defined(MBEDTLS_FS_IO) && defined(MBEDTLS_TEST_HOOKS) */
"""
            )

    _PRINTF_SIMPLE_FORMAT = {
        "int": "%d",
        "long": "%ld",
        "long long": "%lld",
        "size_t": '%"MBEDTLS_PRINTF_SIZET"',
        "unsigned": "0x%08x",
        "unsigned int": "0x%08x",
        "unsigned long": "0x%08lx",
        "unsigned long long": "0x%016llx",
    }

    def _printf_simple_format(self, typ: str) -> Optional[str]:
        """Use this printf format for a value of typ.

        Return None if values of typ need more complex handling.
        """
        return self._PRINTF_SIMPLE_FORMAT.get(typ)

    _PRINTF_TYPE_CAST = {
        "int32_t": "int",
        "uint32_t": "unsigned",
        "uint64_t": "unsigned long long",
    }  # type: Dict[str, str]

    def _printf_type_cast(self, typ: str) -> Optional[str]:
        """Cast values of typ to this type before passing them to printf.

        Return None if values of the given type do not need a cast.
        """
        return self._PRINTF_TYPE_CAST.get(typ)

    _POINTER_TYPE_RE = re.compile(r"\s*\*\Z")

    def _printf_parameters(self, typ: str, var: str) -> Tuple[str, List[str]]:
        """The printf format and arguments for a value of type typ stored in var."""
        expr = var
        base_type = typ
        # For outputs via a pointer, get the value that has been written.
        # Note: we don't support pointers to pointers here.
        pointer_match = self._POINTER_TYPE_RE.search(base_type)
        if pointer_match:
            base_type = base_type[: pointer_match.start(0)]
            expr = "*({})".format(expr)
        # Maybe cast the value to a standard type.
        cast_to = self._printf_type_cast(base_type)
        if cast_to is not None:
            expr = "({}) {}".format(cast_to, expr)
            base_type = cast_to
        # Try standard types.
        fmt = self._printf_simple_format(base_type)
        if fmt is not None:
            return "{}={}".format(var, fmt), [expr]
        raise UnknownTypeForPrintf(typ)

    def _write_function_logging(
        self, out: typing_util.Writable, function: FunctionInfo, argument_names: List[str]
    ) -> None:
        """Write code to log the function's inputs and outputs."""
        formats, values = "%s", ['"' + function.name + '"']
        for arg_info, arg_name in zip(function.arguments, argument_names):
            fmt, vals = self._printf_parameters(arg_info.type, arg_name)
            if fmt:
                formats += " " + fmt
                values += vals
        if not function.returns_void():
            ret_name = self._return_variable_name(function)
            fmt, vals = self._printf_parameters(function.return_type, ret_name)
            if fmt:
                formats += " " + fmt
                values += vals
        out.write(
            """\
#if defined(MBEDTLS_FS_IO) && defined(MBEDTLS_TEST_HOOKS)
    if ({stream}) {{
        mbedtls_fprintf({stream}, "{formats}\\n",
                        {values});
    }}
#endif /* defined(MBEDTLS_FS_IO) && defined(MBEDTLS_TEST_HOOKS) */
""".format(
                stream=self.stream, formats=formats, values=", ".join(values)
            )
        )

    def _write_function_body(
        self, out: typing_util.Writable, function: FunctionInfo, argument_names: List[str]
    ) -> None:
        """Write the body of the wrapper code for the specified function."""
        self._write_function_call(out, function, argument_names)
        self._write_function_logging(out, function, argument_names)
        self._write_function_return(out, function)
