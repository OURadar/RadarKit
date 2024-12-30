r"""Wrapper for RKTypes.h

Generated with:
/usr/local/bin/ctypesgen -I/usr/local/include -Iheaders -Iheaders/RadarKit --no-macro-warnings -lradarkit -o python/src/radarkit/_ctypes_.py headers/RadarKit/RKTypes.h headers/RadarKit/RKMisc.h headers/RadarKit/RKFoundation.h headers/RadarKit/RKConfig.h headers/RadarKit/RKDSP.h headers/RadarKit/RKPulseEngine.h headers/RadarKit/RKFileHeader.h headers/RadarKit/RKScratch.h headers/RadarKit/RKRawDataRecorder.h headers/RadarKit/RKMomentEngine.h headers/RadarKit/RKNoiseEstimator.h headers/RadarKit/RKSweepEngine.h headers/RadarKit/RKPulseRingFilter.h headers/RadarKit/RKMultiLag.h headers/RadarKit/RKPulsePair.h headers/RadarKit/RKPulsePairHop.h headers/RadarKit/RKPulsePairATSR.h headers/RadarKit/RKSpectralMoment.h headers/RadarKit/RKWaveform.h headers/RadarKit/RKProduct.h headers/RadarKit/RKProductFile.h headers/RadarKit/RKTest.h

Do not modify this file.
"""

__docformat__ = "restructuredtext"

# Begin preamble for Python

import ctypes
import sys
from ctypes import *  # noqa: F401, F403

_int_types = (ctypes.c_int16, ctypes.c_int32)
if hasattr(ctypes, "c_int64"):
    # Some builds of ctypes apparently do not have ctypes.c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (ctypes.c_int64,)
for t in _int_types:
    if ctypes.sizeof(t) == ctypes.sizeof(ctypes.c_size_t):
        c_ptrdiff_t = t
del t
del _int_types



class UserString:
    def __init__(self, seq):
        if isinstance(seq, bytes):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq).encode()

    def __bytes__(self):
        return self.data

    def __str__(self):
        return self.data.decode()

    def __repr__(self):
        return repr(self.data)

    def __int__(self):
        return int(self.data.decode())

    def __long__(self):
        return int(self.data.decode())

    def __float__(self):
        return float(self.data.decode())

    def __complex__(self):
        return complex(self.data.decode())

    def __hash__(self):
        return hash(self.data)

    def __le__(self, string):
        if isinstance(string, UserString):
            return self.data <= string.data
        else:
            return self.data <= string

    def __lt__(self, string):
        if isinstance(string, UserString):
            return self.data < string.data
        else:
            return self.data < string

    def __ge__(self, string):
        if isinstance(string, UserString):
            return self.data >= string.data
        else:
            return self.data >= string

    def __gt__(self, string):
        if isinstance(string, UserString):
            return self.data > string.data
        else:
            return self.data > string

    def __eq__(self, string):
        if isinstance(string, UserString):
            return self.data == string.data
        else:
            return self.data == string

    def __ne__(self, string):
        if isinstance(string, UserString):
            return self.data != string.data
        else:
            return self.data != string

    def __contains__(self, char):
        return char in self.data

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.__class__(self.data[index])

    def __getslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, bytes):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other).encode())

    def __radd__(self, other):
        if isinstance(other, bytes):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other).encode() + self.data)

    def __mul__(self, n):
        return self.__class__(self.data * n)

    __rmul__ = __mul__

    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self):
        return self.__class__(self.data.capitalize())

    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))

    def count(self, sub, start=0, end=sys.maxsize):
        return self.data.count(sub, start, end)

    def decode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())

    def encode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())

    def endswith(self, suffix, start=0, end=sys.maxsize):
        return self.data.endswith(suffix, start, end)

    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))

    def find(self, sub, start=0, end=sys.maxsize):
        return self.data.find(sub, start, end)

    def index(self, sub, start=0, end=sys.maxsize):
        return self.data.index(sub, start, end)

    def isalpha(self):
        return self.data.isalpha()

    def isalnum(self):
        return self.data.isalnum()

    def isdecimal(self):
        return self.data.isdecimal()

    def isdigit(self):
        return self.data.isdigit()

    def islower(self):
        return self.data.islower()

    def isnumeric(self):
        return self.data.isnumeric()

    def isspace(self):
        return self.data.isspace()

    def istitle(self):
        return self.data.istitle()

    def isupper(self):
        return self.data.isupper()

    def join(self, seq):
        return self.data.join(seq)

    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))

    def lower(self):
        return self.__class__(self.data.lower())

    def lstrip(self, chars=None):
        return self.__class__(self.data.lstrip(chars))

    def partition(self, sep):
        return self.data.partition(sep)

    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))

    def rfind(self, sub, start=0, end=sys.maxsize):
        return self.data.rfind(sub, start, end)

    def rindex(self, sub, start=0, end=sys.maxsize):
        return self.data.rindex(sub, start, end)

    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))

    def rpartition(self, sep):
        return self.data.rpartition(sep)

    def rstrip(self, chars=None):
        return self.__class__(self.data.rstrip(chars))

    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)

    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)

    def splitlines(self, keepends=0):
        return self.data.splitlines(keepends)

    def startswith(self, prefix, start=0, end=sys.maxsize):
        return self.data.startswith(prefix, start, end)

    def strip(self, chars=None):
        return self.__class__(self.data.strip(chars))

    def swapcase(self):
        return self.__class__(self.data.swapcase())

    def title(self):
        return self.__class__(self.data.title())

    def translate(self, *args):
        return self.__class__(self.data.translate(*args))

    def upper(self):
        return self.__class__(self.data.upper())

    def zfill(self, width):
        return self.__class__(self.data.zfill(width))


class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""

    def __init__(self, string=""):
        self.data = string

    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")

    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + sub + self.data[index + 1 :]

    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + self.data[index + 1 :]

    def __setslice__(self, start, end, sub):
        start = max(start, 0)
        end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start] + sub.data + self.data[end:]
        elif isinstance(sub, bytes):
            self.data = self.data[:start] + sub + self.data[end:]
        else:
            self.data = self.data[:start] + str(sub).encode() + self.data[end:]

    def __delslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]

    def immutable(self):
        return UserString(self.data)

    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, bytes):
            self.data += other
        else:
            self.data += str(other).encode()
        return self

    def __imul__(self, n):
        self.data *= n
        return self


class String(MutableString, ctypes.Union):

    _fields_ = [("raw", ctypes.POINTER(ctypes.c_char)), ("data", ctypes.c_char_p)]

    def __init__(self, obj=b""):
        if isinstance(obj, (bytes, UserString)):
            self.data = bytes(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(ctypes.POINTER(ctypes.c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from bytes
        elif isinstance(obj, bytes):
            return cls(obj)

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj.encode())

        # Convert from c_char_p
        elif isinstance(obj, ctypes.c_char_p):
            return obj

        # Convert from POINTER(ctypes.c_char)
        elif isinstance(obj, ctypes.POINTER(ctypes.c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(ctypes.cast(obj, ctypes.POINTER(ctypes.c_char)))

        # Convert from ctypes.c_char array
        elif isinstance(obj, ctypes.c_char * len(obj)):
            return obj

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)

    from_param = classmethod(from_param)


def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)


# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to ctypes.c_void_p.
def UNCHECKED(type):
    if hasattr(type, "_type_") and isinstance(type._type_, str) and type._type_ != "P":
        return type
    else:
        return ctypes.c_void_p


# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self, func, restype, argtypes, errcheck):
        self.func = func
        self.func.restype = restype
        self.argtypes = argtypes
        if errcheck:
            self.func.errcheck = errcheck

    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func

    def __call__(self, *args):
        fixed_args = []
        i = 0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i += 1
        return self.func(*fixed_args + list(args[i:]))


def ord_if_char(value):
    """
    Simple helper used for casts to simple builtin types:  if the argument is a
    string type, it will be converted to it's ordinal value.

    This function will raise an exception if the argument is string with more
    than one characters.
    """
    return ord(value) if (isinstance(value, bytes) or isinstance(value, str)) else value

# End preamble

_libs = {}
_libdirs = []

# Begin loader

"""
Load libraries - appropriately for all our supported platforms
"""
# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import ctypes
import ctypes.util
import glob
import os.path
import platform
import re
import sys


def _environ_path(name):
    """Split an environment variable into a path-like list elements"""
    if name in os.environ:
        return os.environ[name].split(":")
    return []


class LibraryLoader:
    """
    A base class For loading of libraries ;-)
    Subclasses load libraries for specific platforms.
    """

    # library names formatted specifically for platforms
    name_formats = ["%s"]

    class Lookup:
        """Looking up calling conventions for a platform"""

        mode = ctypes.DEFAULT_MODE

        def __init__(self, path):
            super(LibraryLoader.Lookup, self).__init__()
            self.access = dict(cdecl=ctypes.CDLL(path, self.mode))

        def get(self, name, calling_convention="cdecl"):
            """Return the given name according to the selected calling convention"""
            if calling_convention not in self.access:
                raise LookupError(
                    "Unknown calling convention '{}' for function '{}'".format(
                        calling_convention, name
                    )
                )
            return getattr(self.access[calling_convention], name)

        def has(self, name, calling_convention="cdecl"):
            """Return True if this given calling convention finds the given 'name'"""
            if calling_convention not in self.access:
                return False
            return hasattr(self.access[calling_convention], name)

        def __getattr__(self, name):
            return getattr(self.access["cdecl"], name)

    def __init__(self):
        self.other_dirs = []

    def __call__(self, libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            # noinspection PyBroadException
            try:
                return self.Lookup(path)
            except Exception:  # pylint: disable=broad-except
                pass

        raise ImportError("Could not load %s." % libname)

    def getpaths(self, libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname
        else:
            # search through a prioritized series of locations for the library

            # we first search any specific directories identified by user
            for dir_i in self.other_dirs:
                for fmt in self.name_formats:
                    # dir_i should be absolute already
                    yield os.path.join(dir_i, fmt % libname)

            # check if this code is even stored in a physical file
            try:
                this_file = __file__
            except NameError:
                this_file = None

            # then we search the directory where the generated python interface is stored
            if this_file is not None:
                for fmt in self.name_formats:
                    yield os.path.abspath(os.path.join(os.path.dirname(__file__), fmt % libname))

            # now, use the ctypes tools to try to find the library
            for fmt in self.name_formats:
                path = ctypes.util.find_library(fmt % libname)
                if path:
                    yield path

            # then we search all paths identified as platform-specific lib paths
            for path in self.getplatformpaths(libname):
                yield path

            # Finally, we'll try the users current working directory
            for fmt in self.name_formats:
                yield os.path.abspath(os.path.join(os.path.curdir, fmt % libname))

    def getplatformpaths(self, _libname):  # pylint: disable=no-self-use
        """Return all the library paths available in this platform"""
        return []


# Darwin (Mac OS X)


class DarwinLibraryLoader(LibraryLoader):
    """Library loader for MacOS"""

    name_formats = [
        "lib%s.dylib",
        "lib%s.so",
        "lib%s.bundle",
        "%s.dylib",
        "%s.so",
        "%s.bundle",
        "%s",
    ]

    class Lookup(LibraryLoader.Lookup):
        """
        Looking up library files for this platform (Darwin aka MacOS)
        """

        # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
        # of the default RTLD_LOCAL.  Without this, you end up with
        # libraries not being loadable, resulting in "Symbol not found"
        # errors
        mode = ctypes.RTLD_GLOBAL

    def getplatformpaths(self, libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [fmt % libname for fmt in self.name_formats]

        for directory in self.getdirs(libname):
            for name in names:
                yield os.path.join(directory, name)

    @staticmethod
    def getdirs(libname):
        """Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        """

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [
                os.path.expanduser("~/lib"),
                "/usr/local/lib",
                "/usr/lib",
            ]

        dirs = []

        if "/" in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
            dirs.extend(_environ_path("LD_RUN_PATH"))

        if hasattr(sys, "frozen") and getattr(sys, "frozen") == "macosx_app":
            dirs.append(os.path.join(os.environ["RESOURCEPATH"], "..", "Frameworks"))

        dirs.extend(dyld_fallback_library_path)

        return dirs


# Posix


class PosixLibraryLoader(LibraryLoader):
    """Library loader for POSIX-like systems (including Linux)"""

    _ld_so_cache = None

    _include = re.compile(r"^\s*include\s+(?P<pattern>.*)")

    name_formats = ["lib%s.so", "%s.so", "%s"]

    class _Directories(dict):
        """Deal with directories"""

        def __init__(self):
            dict.__init__(self)
            self.order = 0

        def add(self, directory):
            """Add a directory to our current set of directories"""
            if len(directory) > 1:
                directory = directory.rstrip(os.path.sep)
            # only adds and updates order if exists and not already in set
            if not os.path.exists(directory):
                return
            order = self.setdefault(directory, self.order)
            if order == self.order:
                self.order += 1

        def extend(self, directories):
            """Add a list of directories to our set"""
            for a_dir in directories:
                self.add(a_dir)

        def ordered(self):
            """Sort the list of directories"""
            return (i[0] for i in sorted(self.items(), key=lambda d: d[1]))

    def _get_ld_so_conf_dirs(self, conf, dirs):
        """
        Recursive function to help parse all ld.so.conf files, including proper
        handling of the `include` directive.
        """

        try:
            with open(conf) as fileobj:
                for dirname in fileobj:
                    dirname = dirname.strip()
                    if not dirname:
                        continue

                    match = self._include.match(dirname)
                    if not match:
                        dirs.add(dirname)
                    else:
                        for dir2 in glob.glob(match.group("pattern")):
                            self._get_ld_so_conf_dirs(dir2, dirs)
        except IOError:
            pass

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = self._Directories()
        for name in (
            "LD_LIBRARY_PATH",
            "SHLIB_PATH",  # HP-UX
            "LIBPATH",  # OS/2, AIX
            "LIBRARY_PATH",  # BE/OS
        ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))

        self._get_ld_so_conf_dirs("/etc/ld.so.conf", directories)

        bitage = platform.architecture()[0]

        unix_lib_dirs_list = []
        if bitage.startswith("64"):
            # prefer 64 bit if that is our arch
            unix_lib_dirs_list += ["/lib64", "/usr/lib64"]

        # must include standard libs, since those paths are also used by 64 bit
        # installs
        unix_lib_dirs_list += ["/lib", "/usr/lib"]
        if sys.platform.startswith("linux"):
            # Try and support multiarch work in Ubuntu
            # https://wiki.ubuntu.com/MultiarchSpec
            if bitage.startswith("32"):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ["/lib/i386-linux-gnu", "/usr/lib/i386-linux-gnu"]
            elif bitage.startswith("64"):
                # Assume Intel/AMD x86 compatible
                unix_lib_dirs_list += [
                    "/lib/x86_64-linux-gnu",
                    "/usr/lib/x86_64-linux-gnu",
                ]
            else:
                # guess...
                unix_lib_dirs_list += glob.glob("/lib/*linux-gnu")
        directories.extend(unix_lib_dirs_list)

        cache = {}
        lib_re = re.compile(r"lib(.*)\.s[ol]")
        # ext_re = re.compile(r"\.s[ol]$")
        for our_dir in directories.ordered():
            try:
                for path in glob.glob("%s/*.s[ol]*" % our_dir):
                    file = os.path.basename(path)

                    # Index by filename
                    cache_i = cache.setdefault(file, set())
                    cache_i.add(path)

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        cache_i = cache.setdefault(library, set())
                        cache_i.add(path)
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname, set())
        for i in result:
            # we iterate through all found paths for library, since we may have
            # actually found multiple architectures or other library types that
            # may not load
            yield i


# Windows


class WindowsLibraryLoader(LibraryLoader):
    """Library loader for Microsoft Windows"""

    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll", "%s"]

    class Lookup(LibraryLoader.Lookup):
        """Lookup class for Windows libraries..."""

        def __init__(self, path):
            super(WindowsLibraryLoader.Lookup, self).__init__(path)
            self.access["stdcall"] = ctypes.windll.LoadLibrary(path)


# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin": DarwinLibraryLoader,
    "cygwin": WindowsLibraryLoader,
    "win32": WindowsLibraryLoader,
    "msys": WindowsLibraryLoader,
}

load_library = loaderclass.get(sys.platform, PosixLibraryLoader)()


def add_library_search_dirs(other_dirs):
    """
    Add libraries to search paths.
    If library paths are relative, convert them to absolute with respect to this
    file's directory
    """
    for path in other_dirs:
        if not os.path.isabs(path):
            path = os.path.abspath(path)
        load_library.other_dirs.append(path)


del loaderclass

# End loader

add_library_search_dirs([])

# Begin libraries
_libs["radarkit"] = load_library("radarkit")

# 1 libraries
# End libraries

# No modules

__uint8_t = c_ubyte# /usr/include/x86_64-linux-gnu/bits/types.h: 38

__uint16_t = c_ushort# /usr/include/x86_64-linux-gnu/bits/types.h: 40

__uint32_t = c_uint# /usr/include/x86_64-linux-gnu/bits/types.h: 42

__uint64_t = c_ulong# /usr/include/x86_64-linux-gnu/bits/types.h: 45

__off_t = c_long# /usr/include/x86_64-linux-gnu/bits/types.h: 152

__off64_t = c_long# /usr/include/x86_64-linux-gnu/bits/types.h: 153

__time_t = c_long# /usr/include/x86_64-linux-gnu/bits/types.h: 160

__suseconds_t = c_long# /usr/include/x86_64-linux-gnu/bits/types.h: 162

__syscall_slong_t = c_long# /usr/include/x86_64-linux-gnu/bits/types.h: 197

# /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h: 49
class struct__IO_FILE(Structure):
    pass

FILE = struct__IO_FILE# /usr/include/x86_64-linux-gnu/bits/types/FILE.h: 7

# /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h: 36
class struct__IO_marker(Structure):
    pass

# /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h: 37
class struct__IO_codecvt(Structure):
    pass

# /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h: 38
class struct__IO_wide_data(Structure):
    pass

_IO_lock_t = None# /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h: 43

struct__IO_FILE.__slots__ = [
    '_flags',
    '_IO_read_ptr',
    '_IO_read_end',
    '_IO_read_base',
    '_IO_write_base',
    '_IO_write_ptr',
    '_IO_write_end',
    '_IO_buf_base',
    '_IO_buf_end',
    '_IO_save_base',
    '_IO_backup_base',
    '_IO_save_end',
    '_markers',
    '_chain',
    '_fileno',
    '_flags2',
    '_old_offset',
    '_cur_column',
    '_vtable_offset',
    '_shortbuf',
    '_lock',
    '_offset',
    '_codecvt',
    '_wide_data',
    '_freeres_list',
    '_freeres_buf',
    '__pad5',
    '_mode',
    '_unused2',
]
struct__IO_FILE._fields_ = [
    ('_flags', c_int),
    ('_IO_read_ptr', String),
    ('_IO_read_end', String),
    ('_IO_read_base', String),
    ('_IO_write_base', String),
    ('_IO_write_ptr', String),
    ('_IO_write_end', String),
    ('_IO_buf_base', String),
    ('_IO_buf_end', String),
    ('_IO_save_base', String),
    ('_IO_backup_base', String),
    ('_IO_save_end', String),
    ('_markers', POINTER(struct__IO_marker)),
    ('_chain', POINTER(struct__IO_FILE)),
    ('_fileno', c_int),
    ('_flags2', c_int),
    ('_old_offset', __off_t),
    ('_cur_column', c_ushort),
    ('_vtable_offset', c_char),
    ('_shortbuf', c_char * int(1)),
    ('_lock', POINTER(_IO_lock_t)),
    ('_offset', __off64_t),
    ('_codecvt', POINTER(struct__IO_codecvt)),
    ('_wide_data', POINTER(struct__IO_wide_data)),
    ('_freeres_list', POINTER(struct__IO_FILE)),
    ('_freeres_buf', POINTER(None)),
    ('__pad5', c_size_t),
    ('_mode', c_int),
    ('_unused2', c_char * int((((15 * sizeof(c_int)) - (4 * sizeof(POINTER(None)))) - sizeof(c_size_t)))),
]

# /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h: 8
class struct_timeval(Structure):
    pass

struct_timeval.__slots__ = [
    'tv_sec',
    'tv_usec',
]
struct_timeval._fields_ = [
    ('tv_sec', __time_t),
    ('tv_usec', __suseconds_t),
]

# /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h: 11
class struct_timespec(Structure):
    pass

struct_timespec.__slots__ = [
    'tv_sec',
    'tv_nsec',
]
struct_timespec._fields_ = [
    ('tv_sec', __time_t),
    ('tv_nsec', __syscall_slong_t),
]

__fd_mask = c_long# /usr/include/x86_64-linux-gnu/sys/select.h: 49

# /usr/include/x86_64-linux-gnu/sys/select.h: 70
class struct_anon_8(Structure):
    pass

struct_anon_8.__slots__ = [
    '__fds_bits',
]
struct_anon_8._fields_ = [
    ('__fds_bits', __fd_mask * int((1024 / (8 * (c_int (ord_if_char(sizeof(__fd_mask)))).value)))),
]

fd_set = struct_anon_8# /usr/include/x86_64-linux-gnu/sys/select.h: 70

# /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h: 51
class struct___pthread_internal_list(Structure):
    pass

struct___pthread_internal_list.__slots__ = [
    '__prev',
    '__next',
]
struct___pthread_internal_list._fields_ = [
    ('__prev', POINTER(struct___pthread_internal_list)),
    ('__next', POINTER(struct___pthread_internal_list)),
]

__pthread_list_t = struct___pthread_internal_list# /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h: 55

# /usr/include/x86_64-linux-gnu/bits/struct_mutex.h: 22
class struct___pthread_mutex_s(Structure):
    pass

struct___pthread_mutex_s.__slots__ = [
    '__lock',
    '__count',
    '__owner',
    '__nusers',
    '__kind',
    '__spins',
    '__elision',
    '__list',
]
struct___pthread_mutex_s._fields_ = [
    ('__lock', c_int),
    ('__count', c_uint),
    ('__owner', c_int),
    ('__nusers', c_uint),
    ('__kind', c_int),
    ('__spins', c_short),
    ('__elision', c_short),
    ('__list', __pthread_list_t),
]

pthread_t = c_ulong# /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h: 27

# /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h: 56
class union_pthread_attr_t(Union):
    pass

union_pthread_attr_t.__slots__ = [
    '__size',
    '__align',
]
union_pthread_attr_t._fields_ = [
    ('__size', c_char * int(56)),
    ('__align', c_long),
]

pthread_attr_t = union_pthread_attr_t# /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h: 62

# /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h: 72
class union_anon_14(Union):
    pass

union_anon_14.__slots__ = [
    '__data',
    '__size',
    '__align',
]
union_anon_14._fields_ = [
    ('__data', struct___pthread_mutex_s),
    ('__size', c_char * int(40)),
    ('__align', c_long),
]

pthread_mutex_t = union_anon_14# /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h: 72

uint8_t = __uint8_t# /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h: 24

uint16_t = __uint16_t# /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h: 25

uint32_t = __uint32_t# /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h: 26

uint64_t = __uint64_t# /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h: 27

# /usr/include/x86_64-linux-gnu/bits/mathcalls.h: 107
if _libs["radarkit"].has("log10f", "cdecl"):
    log10f = _libs["radarkit"].get("log10f", "cdecl")
    log10f.argtypes = [c_float]
    log10f.restype = c_float

# /usr/include/x86_64-linux-gnu/bits/mathcalls.h: 162
if _libs["radarkit"].has("fabsf", "cdecl"):
    fabsf = _libs["radarkit"].get("fabsf", "cdecl")
    fabsf.argtypes = [c_float]
    fabsf.restype = c_float

# /usr/include/x86_64-linux-gnu/bits/mathcalls.h: 165
if _libs["radarkit"].has("floorf", "cdecl"):
    floorf = _libs["radarkit"].get("floorf", "cdecl")
    floorf.argtypes = [c_float]
    floorf.restype = c_float

# /usr/include/x86_64-linux-gnu/bits/mathcalls.h: 301
if _libs["radarkit"].has("roundf", "cdecl"):
    roundf = _libs["radarkit"].get("roundf", "cdecl")
    roundf.argtypes = [c_float]
    roundf.restype = c_float

# /usr/include/x86_64-linux-gnu/bits/semaphore.h: 39
class union_anon_64(Union):
    pass

union_anon_64.__slots__ = [
    '__size',
    '__align',
]
union_anon_64._fields_ = [
    ('__size', c_char * int(32)),
    ('__align', c_long),
]

sem_t = union_anon_64# /usr/include/x86_64-linux-gnu/bits/semaphore.h: 39

# /usr/include/errno.h: 37
if _libs["radarkit"].has("__errno_location", "cdecl"):
    __errno_location = _libs["radarkit"].get("__errno_location", "cdecl")
    __errno_location.argtypes = []
    __errno_location.restype = POINTER(c_int)

RKByte = uint8_t# RKTypes.h: 191

RKFloat = c_float# RKTypes.h: 192

RKResult = c_ptrdiff_t# RKTypes.h: 193

RKBuffer = POINTER(uint8_t)# RKTypes.h: 194

RKTransceiver = POINTER(None)# RKTypes.h: 195

RKPedestal = POINTER(None)# RKTypes.h: 196

RKHealthRelay = POINTER(None)# RKTypes.h: 197

RKMasterController = POINTER(None)# RKTypes.h: 198

RKUserResource = POINTER(None)# RKTypes.h: 199

RKUserModule = POINTER(None)# RKTypes.h: 200

RKCompressor = POINTER(None)# RKTypes.h: 201

RKName = c_char * int(128)# RKTypes.h: 202

RKChildName = c_char * int(160)# RKTypes.h: 203

RKShortName = c_char * int(20)# RKTypes.h: 204

RKCommand = c_char * int(512)# RKTypes.h: 205

RKProductId = uint8_t# RKTypes.h: 206

RKIdentifier = uint64_t# RKTypes.h: 207

RKConst = c_float# RKTypes.h: 208

# RKTypes.h: 222
class struct_rk_int16c(Structure):
    pass

struct_rk_int16c._pack_ = 1
struct_rk_int16c.__slots__ = [
    'i',
    'q',
]
struct_rk_int16c._fields_ = [
    ('i', c_int16),
    ('q', c_int16),
]

RKInt16C = struct_rk_int16c# RKTypes.h: 222

# RKTypes.h: 230
class struct_rk_complex(Structure):
    pass

struct_rk_complex._pack_ = 1
struct_rk_complex.__slots__ = [
    'i',
    'q',
]
struct_rk_complex._fields_ = [
    ('i', RKFloat),
    ('q', RKFloat),
]

RKComplex = struct_rk_complex# RKTypes.h: 230

# RKTypes.h: 238
class struct_rk_iqz(Structure):
    pass

struct_rk_iqz._pack_ = 1
struct_rk_iqz.__slots__ = [
    'i',
    'q',
]
struct_rk_iqz._fields_ = [
    ('i', POINTER(RKFloat)),
    ('q', POINTER(RKFloat)),
]

RKIQZ = struct_rk_iqz# RKTypes.h: 238

# RKTypes.h: 248
class struct_rk_modulo_path(Structure):
    pass

struct_rk_modulo_path._pack_ = 1
struct_rk_modulo_path.__slots__ = [
    'origin',
    'length',
    'modulo',
    'planIndex',
]
struct_rk_modulo_path._fields_ = [
    ('origin', uint32_t),
    ('length', uint32_t),
    ('modulo', uint32_t),
    ('planIndex', uint32_t),
]

RKModuloPath = struct_rk_modulo_path# RKTypes.h: 248

# RKTypes.h: 254
class struct_anon_65(Structure):
    pass

struct_anon_65._pack_ = 1
struct_anon_65.__slots__ = [
    'byte',
]
struct_anon_65._fields_ = [
    ('byte', RKByte * int(4)),
]

# RKTypes.h: 255
class struct_anon_66(Structure):
    pass

struct_anon_66._pack_ = 1
struct_anon_66.__slots__ = [
    'u8_1',
    'u8_2',
    'u8_3',
    'u8_4',
]
struct_anon_66._fields_ = [
    ('u8_1', uint8_t),
    ('u8_2', uint8_t),
    ('u8_3', uint8_t),
    ('u8_4', uint8_t),
]

# RKTypes.h: 256
class struct_anon_67(Structure):
    pass

struct_anon_67._pack_ = 1
struct_anon_67.__slots__ = [
    'i8_1',
    'i8_2',
    'i8_3',
    'i8_4',
]
struct_anon_67._fields_ = [
    ('i8_1', c_int8),
    ('i8_2', c_int8),
    ('i8_3', c_int8),
    ('i8_4', c_int8),
]

# RKTypes.h: 257
class struct_anon_68(Structure):
    pass

struct_anon_68._pack_ = 1
struct_anon_68.__slots__ = [
    'u16_1',
    'u16_2',
]
struct_anon_68._fields_ = [
    ('u16_1', uint16_t),
    ('u16_2', uint16_t),
]

# RKTypes.h: 258
class struct_anon_69(Structure):
    pass

struct_anon_69._pack_ = 1
struct_anon_69.__slots__ = [
    'i16_1',
    'i16_2',
]
struct_anon_69._fields_ = [
    ('i16_1', c_int16),
    ('i16_2', c_int16),
]

# RKTypes.h: 259
class struct_anon_70(Structure):
    pass

struct_anon_70._pack_ = 1
struct_anon_70.__slots__ = [
    'u32',
]
struct_anon_70._fields_ = [
    ('u32', uint32_t),
]

# RKTypes.h: 260
class struct_anon_71(Structure):
    pass

struct_anon_71._pack_ = 1
struct_anon_71.__slots__ = [
    'i32',
]
struct_anon_71._fields_ = [
    ('i32', c_int32),
]

# RKTypes.h: 261
class struct_anon_72(Structure):
    pass

struct_anon_72._pack_ = 1
struct_anon_72.__slots__ = [
    'f',
]
struct_anon_72._fields_ = [
    ('f', c_float),
]

# RKTypes.h: 262
class union_rk_four_byte(Union):
    pass

union_rk_four_byte._pack_ = 1
union_rk_four_byte.__slots__ = [
    'unnamed_1',
    'unnamed_2',
    'unnamed_3',
    'unnamed_4',
    'unnamed_5',
    'unnamed_6',
    'unnamed_7',
    'unnamed_8',
]
union_rk_four_byte._anonymous_ = [
    'unnamed_1',
    'unnamed_2',
    'unnamed_3',
    'unnamed_4',
    'unnamed_5',
    'unnamed_6',
    'unnamed_7',
    'unnamed_8',
]
union_rk_four_byte._fields_ = [
    ('unnamed_1', struct_anon_65),
    ('unnamed_2', struct_anon_66),
    ('unnamed_3', struct_anon_67),
    ('unnamed_4', struct_anon_68),
    ('unnamed_5', struct_anon_69),
    ('unnamed_6', struct_anon_70),
    ('unnamed_7', struct_anon_71),
    ('unnamed_8', struct_anon_72),
]

RKFourByte = union_rk_four_byte# RKTypes.h: 262

# RKTypes.h: 268
class struct_anon_73(Structure):
    pass

struct_anon_73._pack_ = 1
struct_anon_73.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_73._fields_ = [
    ('m', uint16_t, 10),
    ('e', uint8_t, 5),
    ('s', uint8_t, 1),
]

# RKTypes.h: 275
class union_rk_half_float_t(Union):
    pass

union_rk_half_float_t._pack_ = 1
union_rk_half_float_t.__slots__ = [
    'unnamed_1',
    'bytes',
    'word',
]
union_rk_half_float_t._anonymous_ = [
    'unnamed_1',
]
union_rk_half_float_t._fields_ = [
    ('unnamed_1', struct_anon_73),
    ('bytes', uint8_t * int(2)),
    ('word', uint16_t),
]

RKWordFloat16 = union_rk_half_float_t# RKTypes.h: 275

# RKTypes.h: 281
class struct_anon_74(Structure):
    pass

struct_anon_74._pack_ = 1
struct_anon_74.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_74._fields_ = [
    ('m', uint32_t, 23),
    ('e', uint8_t, 8),
    ('s', uint8_t, 1),
]

# RKTypes.h: 289
class union_rk_single_float_t(Union):
    pass

union_rk_single_float_t._pack_ = 1
union_rk_single_float_t.__slots__ = [
    'unnamed_1',
    'bytes',
    'word',
    'value',
]
union_rk_single_float_t._anonymous_ = [
    'unnamed_1',
]
union_rk_single_float_t._fields_ = [
    ('unnamed_1', struct_anon_74),
    ('bytes', uint8_t * int(4)),
    ('word', uint32_t),
    ('value', c_float),
]

RKWordFloat32 = union_rk_single_float_t# RKTypes.h: 289

# RKTypes.h: 295
class struct_anon_75(Structure):
    pass

struct_anon_75._pack_ = 1
struct_anon_75.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_75._fields_ = [
    ('m', uint64_t, 52),
    ('e', uint16_t, 11),
    ('s', uint8_t, 1),
]

# RKTypes.h: 303
class union_rk_double_float_t(Union):
    pass

union_rk_double_float_t._pack_ = 1
union_rk_double_float_t.__slots__ = [
    'unnamed_1',
    'bytes',
    'word',
    'value',
]
union_rk_double_float_t._anonymous_ = [
    'unnamed_1',
]
union_rk_double_float_t._fields_ = [
    ('unnamed_1', struct_anon_75),
    ('bytes', uint8_t * int(8)),
    ('word', uint64_t),
    ('value', c_double),
]

RKWordFloat64 = union_rk_double_float_t# RKTypes.h: 303

# RKTypes.h: 306
class struct_anon_76(Structure):
    pass

struct_anon_76._pack_ = 1
struct_anon_76.__slots__ = [
    'name',
    'origin',
    'length',
    'inputOrigin',
    'outputOrigin',
    'maxDataLength',
    'subCarrierFrequency',
    'sensitivityGain',
    'filterGain',
    'fullScale',
    'lowerBoundFrequency',
    'upperBoundFrequency',
]
struct_anon_76._fields_ = [
    ('name', uint32_t),
    ('origin', uint32_t),
    ('length', uint32_t),
    ('inputOrigin', uint32_t),
    ('outputOrigin', uint32_t),
    ('maxDataLength', uint32_t),
    ('subCarrierFrequency', c_float),
    ('sensitivityGain', c_float),
    ('filterGain', c_float),
    ('fullScale', c_float),
    ('lowerBoundFrequency', c_float),
    ('upperBoundFrequency', c_float),
]

# RKTypes.h: 321
class union_rk_filter_anchor(Union):
    pass

union_rk_filter_anchor._pack_ = 1
union_rk_filter_anchor.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_filter_anchor._anonymous_ = [
    'unnamed_1',
]
union_rk_filter_anchor._fields_ = [
    ('unnamed_1', struct_anon_76),
    ('bytes', c_char * int(64)),
]

RKFilterAnchor = union_rk_filter_anchor# RKTypes.h: 321

RKFilterAnchorGroup = RKFilterAnchor * int(8)# RKTypes.h: 323

enum_anon_77 = c_int# RKTypes.h: 417

RKResultSuccess = 0# RKTypes.h: 417

RKResultTooBig = (RKResultSuccess + 1)# RKTypes.h: 417

RKResultTimeout = (RKResultTooBig + 1)# RKTypes.h: 417

RKResultNullInput = (RKResultTimeout + 1)# RKTypes.h: 417

RKResultEngineNotWired = (RKResultNullInput + 1)# RKTypes.h: 417

RKResultEngineNotActive = (RKResultEngineNotWired + 1)# RKTypes.h: 417

RKResultIncompleteSend = (RKResultEngineNotActive + 1)# RKTypes.h: 417

RKResultIncompleteReceive = (RKResultIncompleteSend + 1)# RKTypes.h: 417

RKResultIncompleteTransceiver = (RKResultIncompleteReceive + 1)# RKTypes.h: 417

RKResultIncompletePedestal = (RKResultIncompleteTransceiver + 1)# RKTypes.h: 417

RKResultIncompleteHealthRelay = (RKResultIncompletePedestal + 1)# RKTypes.h: 417

RKResultIncompleteControl = (RKResultIncompleteHealthRelay + 1)# RKTypes.h: 417

RKResultIncompleteWaveformCalibration = (RKResultIncompleteControl + 1)# RKTypes.h: 417

RKResultIncompleteProductDescription = (RKResultIncompleteWaveformCalibration + 1)# RKTypes.h: 417

RKResultIncompleteScanDescription = (RKResultIncompleteProductDescription + 1)# RKTypes.h: 417

RKResultErrorCreatingOperatorRoutine = (RKResultIncompleteScanDescription + 1)# RKTypes.h: 417

RKResultErrorCreatingOperatorCommandRoutine = (RKResultErrorCreatingOperatorRoutine + 1)# RKTypes.h: 417

RKResultErrorCreatingClientRoutine = (RKResultErrorCreatingOperatorCommandRoutine + 1)# RKTypes.h: 417

RKResultSDToFDError = (RKResultErrorCreatingClientRoutine + 1)# RKTypes.h: 417

RKResultNoPulseBuffer = (RKResultSDToFDError + 1)# RKTypes.h: 417

RKResultNoRayBuffer = (RKResultNoPulseBuffer + 1)# RKTypes.h: 417

RKResultNoPulseCompressionEngine = (RKResultNoRayBuffer + 1)# RKTypes.h: 417

RKResultNoPulseRingEngine = (RKResultNoPulseCompressionEngine + 1)# RKTypes.h: 417

RKResultNoMomentEngine = (RKResultNoPulseRingEngine + 1)# RKTypes.h: 417

RKResultFailedToStartCompressionCore = (RKResultNoMomentEngine + 1)# RKTypes.h: 417

RKResultFailedToStartRingCore = (RKResultFailedToStartCompressionCore + 1)# RKTypes.h: 417

RKResultFailedToStartPulseWatcher = (RKResultFailedToStartRingCore + 1)# RKTypes.h: 417

RKResultFailedToStartRingPulseWatcher = (RKResultFailedToStartPulseWatcher + 1)# RKTypes.h: 417

RKResultFailedToInitiateSemaphore = (RKResultFailedToStartRingPulseWatcher + 1)# RKTypes.h: 417

RKResultFailedToRetrieveSemaphore = (RKResultFailedToInitiateSemaphore + 1)# RKTypes.h: 417

RKResultFailedToAllocateFFTSpace = (RKResultFailedToRetrieveSemaphore + 1)# RKTypes.h: 417

RKResultFailedToAllocateFilter = (RKResultFailedToAllocateFFTSpace + 1)# RKTypes.h: 417

RKResultFailedToAllocateDutyCycleBuffer = (RKResultFailedToAllocateFilter + 1)# RKTypes.h: 417

RKResultFailedToAllocateScratchSpace = (RKResultFailedToAllocateDutyCycleBuffer + 1)# RKTypes.h: 417

RKResultFailedToSetWaveform = (RKResultFailedToAllocateScratchSpace + 1)# RKTypes.h: 417

RKResultFailedToSetFilter = (RKResultFailedToSetWaveform + 1)# RKTypes.h: 417

RKResultEngineDeactivatedMultipleTimes = (RKResultFailedToSetFilter + 1)# RKTypes.h: 417

RKResultFailedToStartMomentCore = (RKResultEngineDeactivatedMultipleTimes + 1)# RKTypes.h: 417

RKResultFailedToStartPulseGatherer = (RKResultFailedToStartMomentCore + 1)# RKTypes.h: 417

RKResultUnableToChangeCoreCounts = (RKResultFailedToStartPulseGatherer + 1)# RKTypes.h: 417

RKResultFailedToStartPedestalWorker = (RKResultUnableToChangeCoreCounts + 1)# RKTypes.h: 417

RKResultFailedToGetVacantPosition = (RKResultFailedToStartPedestalWorker + 1)# RKTypes.h: 417

RKResultFailedToGetVacantHealth = (RKResultFailedToGetVacantPosition + 1)# RKTypes.h: 417

RKResultFailedToStartRayGatherer = (RKResultFailedToGetVacantHealth + 1)# RKTypes.h: 417

RKResultFailedToStartHealthWorker = (RKResultFailedToStartRayGatherer + 1)# RKTypes.h: 417

RKResultFailedToStartPulseRecorder = (RKResultFailedToStartHealthWorker + 1)# RKTypes.h: 417

RKResultFailedToStartPedestalMonitor = (RKResultFailedToStartPulseRecorder + 1)# RKTypes.h: 417

RKResultFailedToStartpedestalVcpEngine = (RKResultFailedToStartPedestalMonitor + 1)# RKTypes.h: 417

RKResultFailedToStartFileManager = (RKResultFailedToStartpedestalVcpEngine + 1)# RKTypes.h: 417

RKResultFailedToStartFileRemover = (RKResultFailedToStartFileManager + 1)# RKTypes.h: 417

RKResultFailedToStartTransceiver = (RKResultFailedToStartFileRemover + 1)# RKTypes.h: 417

RKResultFailedToStartPedestal = (RKResultFailedToStartTransceiver + 1)# RKTypes.h: 417

RKResultFailedToStartHealthRelay = (RKResultFailedToStartPedestal + 1)# RKTypes.h: 417

RKResultPreferenceFileNotFound = (RKResultFailedToStartHealthRelay + 1)# RKTypes.h: 417

RKResultPreferenceKeywordNotFound = (RKResultPreferenceFileNotFound + 1)# RKTypes.h: 417

RKResultFailedToMeasureNoise = (RKResultPreferenceKeywordNotFound + 1)# RKTypes.h: 417

RKResultFailedToEstimateNoise = (RKResultFailedToMeasureNoise + 1)# RKTypes.h: 417

RKResultFailedToCreateFileRemover = (RKResultFailedToEstimateNoise + 1)# RKTypes.h: 417

RKResultFileManagerBufferNotResuable = (RKResultFailedToCreateFileRemover + 1)# RKTypes.h: 417

RKResultInvalidMomentParameters = (RKResultFileManagerBufferNotResuable + 1)# RKTypes.h: 417

RKResultFailedToCreateUnitWorker = (RKResultInvalidMomentParameters + 1)# RKTypes.h: 417

RKResultFailedToStartHostWatcher = (RKResultFailedToCreateUnitWorker + 1)# RKTypes.h: 417

RKResultFailedToStartHostPinger = (RKResultFailedToStartHostWatcher + 1)# RKTypes.h: 417

RKResultFailedToExecuteCommand = (RKResultFailedToStartHostPinger + 1)# RKTypes.h: 417

RKResultFailedToSetVCP = (RKResultFailedToExecuteCommand + 1)# RKTypes.h: 417

RKResultFailedToAddHost = (RKResultFailedToSetVCP + 1)# RKTypes.h: 417

RKResultFailedToFindProductId = (RKResultFailedToAddHost + 1)# RKTypes.h: 417

RKResultFailedToOpenFileForProduct = (RKResultFailedToFindProductId + 1)# RKTypes.h: 417

RKResultClientNotConnected = (RKResultFailedToOpenFileForProduct + 1)# RKTypes.h: 417

RKResultFileManagerInconsistentFolder = (RKResultClientNotConnected + 1)# RKTypes.h: 417

RKResultFailedToExpandWaveform = (RKResultFileManagerInconsistentFolder + 1)# RKTypes.h: 417

RKResultFailedToOpenFileForWriting = (RKResultFailedToExpandWaveform + 1)# RKTypes.h: 417

RKResultFailedToStandardizeProduct = (RKResultFailedToOpenFileForWriting + 1)# RKTypes.h: 417

RKResultRadarNotLive = (RKResultFailedToStandardizeProduct + 1)# RKTypes.h: 417

RKResultRawDataTypeUndefined = (RKResultRadarNotLive + 1)# RKTypes.h: 417

RKResultNothingToRead = (RKResultRawDataTypeUndefined + 1)# RKTypes.h: 417

RKResultProductDescriptionNotSet = (RKResultNothingToRead + 1)# RKTypes.h: 417

RKResultProductDimensionsNotSet = (RKResultProductDescriptionNotSet + 1)# RKTypes.h: 417

RKResultProductStartTimeNotSet = (RKResultProductDimensionsNotSet + 1)# RKTypes.h: 417

RKResultProductGateSizeNotSet = (RKResultProductStartTimeNotSet + 1)# RKTypes.h: 417

RKResultFilenameHasNoPrefix = (RKResultProductGateSizeNotSet + 1)# RKTypes.h: 417

RKResultFilenameHasBadDate = (RKResultFilenameHasNoPrefix + 1)# RKTypes.h: 417

RKResultFilenameHasBadTime = (RKResultFilenameHasBadDate + 1)# RKTypes.h: 417

RKResultFilenameHasBadScan = (RKResultFilenameHasBadTime + 1)# RKTypes.h: 417

RKResultFilenameHasNoProduct = (RKResultFilenameHasBadScan + 1)# RKTypes.h: 417

RKResultFailedToOpenFile = (RKResultFilenameHasNoProduct + 1)# RKTypes.h: 417

RKResultNoRadar = (RKResultFailedToOpenFile + 1)# RKTypes.h: 417

RKResultCount = (RKResultNoRadar + 1)# RKTypes.h: 417

enum_RKEngineColor = c_int# RKTypes.h: 423

RKEngineColorCommandCenter = 14# RKTypes.h: 423

RKEngineColorRadarHubReporter = 10# RKTypes.h: 423

RKEngineColorPulseCompressionEngine = 7# RKTypes.h: 423

RKEngineColorPulseRingFilterEngine = 3# RKTypes.h: 423

RKEngineColorPositionEngine = 4# RKTypes.h: 423

RKEngineColorSteerEngine = 3# RKTypes.h: 423

RKEngineColorMomentEngine = 15# RKTypes.h: 423

RKEngineColorHealthEngine = 1# RKTypes.h: 423

RKEngineColorDataRecorder = 12# RKTypes.h: 423

RKEngineColorSweepEngine = 18# RKTypes.h: 423

RKEngineColorHealthLogger = 5# RKTypes.h: 423

RKEngineColorFileManager = 2# RKTypes.h: 423

RKEngineColorTransceiver = 17# RKTypes.h: 423

RKEngineColorPedestalRelayPedzy = 15# RKTypes.h: 423

RKEngineColorHealthRelayTweeta = 0# RKTypes.h: 423

RKEngineColorHealthRelayNaveen = 11# RKTypes.h: 423

RKEngineColorRadarRelay = 17# RKTypes.h: 423

RKEngineColorHostMonitor = 16# RKTypes.h: 423

RKEngineColorClock = 19# RKTypes.h: 423

RKEngineColorMisc = 20# RKTypes.h: 423

RKEngineColorEngineMonitor = 19# RKTypes.h: 423

RKEngineColorConfig = 6# RKTypes.h: 423

RKEngineColorFFTModule = 19# RKTypes.h: 423

RKEngineColorWebSocket = 8# RKTypes.h: 423

RKValueType = uint32_t# RKTypes.h: 450

enum_anon_78 = c_int# RKTypes.h: 451

RKValueTypeNull = 0# RKTypes.h: 451

RKValueTypeBool = (RKValueTypeNull + 1)# RKTypes.h: 451

RKValueTypeInt = (RKValueTypeBool + 1)# RKTypes.h: 451

RKValueTypeLong = (RKValueTypeInt + 1)# RKTypes.h: 451

RKValueTypeInt8 = (RKValueTypeLong + 1)# RKTypes.h: 451

RKValueTypeInt16 = (RKValueTypeInt8 + 1)# RKTypes.h: 451

RKValueTypeInt32 = (RKValueTypeInt16 + 1)# RKTypes.h: 451

RKValueTypeInt64 = (RKValueTypeInt32 + 1)# RKTypes.h: 451

RKValueTypeSSize = (RKValueTypeInt64 + 1)# RKTypes.h: 451

RKValueTypeUInt = (RKValueTypeSSize + 1)# RKTypes.h: 451

RKValueTypeULong = (RKValueTypeUInt + 1)# RKTypes.h: 451

RKValueTypeUInt8 = (RKValueTypeULong + 1)# RKTypes.h: 451

RKValueTypeUInt16 = (RKValueTypeUInt8 + 1)# RKTypes.h: 451

RKValueTypeUInt32 = (RKValueTypeUInt16 + 1)# RKTypes.h: 451

RKValueTypeUInt64 = (RKValueTypeUInt32 + 1)# RKTypes.h: 451

RKValueTypeIntInHex = (RKValueTypeUInt64 + 1)# RKTypes.h: 451

RKValueTypeLongInHex = (RKValueTypeIntInHex + 1)# RKTypes.h: 451

RKValueTypeInt8InHex = (RKValueTypeLongInHex + 1)# RKTypes.h: 451

RKValueTypeInt16InHex = (RKValueTypeInt8InHex + 1)# RKTypes.h: 451

RKValueTypeInt32InHex = (RKValueTypeInt16InHex + 1)# RKTypes.h: 451

RKValueTypeInt64InHex = (RKValueTypeInt32InHex + 1)# RKTypes.h: 451

RKValueTypeSSizeInHex = (RKValueTypeInt64InHex + 1)# RKTypes.h: 451

RKValueTypeUIntInHex = (RKValueTypeSSizeInHex + 1)# RKTypes.h: 451

RKValueTypeULongInHex = (RKValueTypeUIntInHex + 1)# RKTypes.h: 451

RKValueTypeUInt8InHex = (RKValueTypeULongInHex + 1)# RKTypes.h: 451

RKValueTypeUInt16InHex = (RKValueTypeUInt8InHex + 1)# RKTypes.h: 451

RKValueTypeUInt32InHex = (RKValueTypeUInt16InHex + 1)# RKTypes.h: 451

RKValueTypeUInt64InHex = (RKValueTypeUInt32InHex + 1)# RKTypes.h: 451

RKValueTypeSize = (RKValueTypeUInt64InHex + 1)# RKTypes.h: 451

RKValueTypeFloat = (RKValueTypeSize + 1)# RKTypes.h: 451

RKValueTypeDouble = (RKValueTypeFloat + 1)# RKTypes.h: 451

RKValueTypeString = (RKValueTypeDouble + 1)# RKTypes.h: 451

RKValueTypeNumericString = (RKValueTypeString + 1)# RKTypes.h: 451

RKValueTypeFloatWithOneDecimals = (RKValueTypeNumericString + 1)# RKTypes.h: 451

RKValueTypeFloatWithTwoDecimals = (RKValueTypeFloatWithOneDecimals + 1)# RKTypes.h: 451

RKValueTypeFloatWithThreeDecimals = (RKValueTypeFloatWithTwoDecimals + 1)# RKTypes.h: 451

RKValueTypeFloatWithFourDecimals = (RKValueTypeFloatWithThreeDecimals + 1)# RKTypes.h: 451

RKValueTypeFloatWithFiveDecimals = (RKValueTypeFloatWithFourDecimals + 1)# RKTypes.h: 451

RKValueTypeFloatWithSixDecimals = (RKValueTypeFloatWithFiveDecimals + 1)# RKTypes.h: 451

RKValueTYpeFloatMultipliedBy1k = (RKValueTypeFloatWithSixDecimals + 1)# RKTypes.h: 451

RKValueTYpeFloatMultipliedBy1M = (RKValueTYpeFloatMultipliedBy1k + 1)# RKTypes.h: 451

RKValueTYpeFloatDividedBy1k = (RKValueTYpeFloatMultipliedBy1M + 1)# RKTypes.h: 451

RKValueTYpeFloatDividedBy1M = (RKValueTYpeFloatDividedBy1k + 1)# RKTypes.h: 451

RKValueTypeDoubleWithOneDecimals = (RKValueTYpeFloatDividedBy1M + 1)# RKTypes.h: 451

RKValueTypeDoubleWithTwoDecimals = (RKValueTypeDoubleWithOneDecimals + 1)# RKTypes.h: 451

RKValueTypeDoubleWithThreeDecimals = (RKValueTypeDoubleWithTwoDecimals + 1)# RKTypes.h: 451

RKValueTypeDoubleWithFourDecimals = (RKValueTypeDoubleWithThreeDecimals + 1)# RKTypes.h: 451

RKValueTypeDoubleWithFiveDecimals = (RKValueTypeDoubleWithFourDecimals + 1)# RKTypes.h: 451

RKValueTypeDoubleWithSixDecimals = (RKValueTypeDoubleWithFiveDecimals + 1)# RKTypes.h: 451

RKValueTYpeDoubleMultipliedBy1k = (RKValueTypeDoubleWithSixDecimals + 1)# RKTypes.h: 451

RKValueTYpeDoubleMultipliedBy1M = (RKValueTYpeDoubleMultipliedBy1k + 1)# RKTypes.h: 451

RKValueTYpeDoubleDividedBy1k = (RKValueTYpeDoubleMultipliedBy1M + 1)# RKTypes.h: 451

RKValueTYpeDoubleDividedBy1M = (RKValueTYpeDoubleDividedBy1k + 1)# RKTypes.h: 451

RKValueTypeProductId = RKValueTypeInt8# RKTypes.h: 451

RKValueTypeIdentifier = RKValueTypeUInt64# RKTypes.h: 451

RKValueTypeDictionary = (RKValueTypeIdentifier + 1)# RKTypes.h: 451

RKValueTypeArray = (RKValueTypeDictionary + 1)# RKTypes.h: 451

RKValueTypeVariable = (RKValueTypeArray + 1)# RKTypes.h: 451

RKPositionFlag = uint32_t# RKTypes.h: 512

enum_anon_79 = c_int# RKTypes.h: 513

RKPositionFlagVacant = 0# RKTypes.h: 513

RKPositionFlagAzimuthEnabled = 1# RKTypes.h: 513

RKPositionFlagAzimuthSafety = (1 << 1)# RKTypes.h: 513

RKPositionFlagAzimuthError = (1 << 2)# RKTypes.h: 513

RKPositionFlagAzimuthSweep = (1 << 8)# RKTypes.h: 513

RKPositionFlagAzimuthPoint = (1 << 9)# RKTypes.h: 513

RKPositionFlagAzimuthComplete = (1 << 10)# RKTypes.h: 513

RKPositionFlagElevationEnabled = (1 << 16)# RKTypes.h: 513

RKPositionFlagElevationSafety = (1 << 17)# RKTypes.h: 513

RKPositionFlagElevationError = (1 << 18)# RKTypes.h: 513

RKPositionFlagElevationSweep = (1 << 24)# RKTypes.h: 513

RKPositionFlagElevationPoint = (1 << 25)# RKTypes.h: 513

RKPositionFlagElevationComplete = (1 << 26)# RKTypes.h: 513

RKPositionFlagHardwareMask = 0x0FFFFFFF# RKTypes.h: 513

RKPositionFlagScanActive = (1 << 28)# RKTypes.h: 513

RKPositionFlagVCPActive = (1 << 29)# RKTypes.h: 513

RKPositionFlagUsed = (1 << 30)# RKTypes.h: 513

RKPositionFlagReady = (1 << 31)# RKTypes.h: 513

RKPositionFlagAzimuthModeMask = (RKPositionFlagAzimuthSweep | RKPositionFlagAzimuthPoint)# RKTypes.h: 513

RKPositionFlagElevationModeMask = (RKPositionFlagElevationSweep | RKPositionFlagElevationPoint)# RKTypes.h: 513

RKPositionFlagScanModeMask = (RKPositionFlagAzimuthModeMask | RKPositionFlagElevationModeMask)# RKTypes.h: 513

RKSignalLocale = uint8_t# RKTypes.h: 537

enum_anon_80 = c_int# RKTypes.h: 538

RKSignalLocaleNormal = 0# RKTypes.h: 538

RKSignalLocaleFirstNyquist = (RKSignalLocaleNormal + 1)# RKTypes.h: 538

RKHeadingType = uint32_t# RKTypes.h: 543

enum_anon_81 = c_int# RKTypes.h: 544

RKHeadingTypeNormal = 0# RKTypes.h: 544

RKHeadingTypeAdd90 = (RKHeadingTypeNormal + 1)# RKTypes.h: 544

RKHeadingTypeAdd180 = (RKHeadingTypeAdd90 + 1)# RKTypes.h: 544

RKHeadingTypeAdd270 = (RKHeadingTypeAdd180 + 1)# RKTypes.h: 544

RKStatusFlag = uint32_t# RKTypes.h: 551

enum_anon_82 = c_int# RKTypes.h: 552

RKStatusFlagVacant = 0# RKTypes.h: 552

RKStatusFlagReady = 1# RKTypes.h: 552

RKHealthFlag = uint32_t# RKTypes.h: 557

enum_anon_83 = c_int# RKTypes.h: 558

RKHealthFlagVacant = 0# RKTypes.h: 558

RKHealthFlagReady = 1# RKTypes.h: 558

RKHealthFlagUsed = (1 << 1)# RKTypes.h: 558

RKMarker = uint32_t# RKTypes.h: 564

enum_anon_84 = c_int# RKTypes.h: 565

RKMarkerNull = 0# RKTypes.h: 565

RKMarkerSweepMiddle = 1# RKTypes.h: 565

RKMarkerSweepBegin = (1 << 1)# RKTypes.h: 565

RKMarkerSweepEnd = (1 << 2)# RKTypes.h: 565

RKMarkerVolumeBegin = (1 << 3)# RKTypes.h: 565

RKMarkerVolumeEnd = (1 << 4)# RKTypes.h: 565

RKMarkerScanTypeMask = 0x60# RKTypes.h: 565

RKMarkerScanTypeUnknown = (0 << 5)# RKTypes.h: 565

RKMarkerScanTypePPI = (1 << 5)# RKTypes.h: 565

RKMarkerScanTypeRHI = (2 << 5)# RKTypes.h: 565

RKMarkerScanTytpePoint = (3 << 5)# RKTypes.h: 565

RKMarkerMemoryManagement = (1 << 7)# RKTypes.h: 565

RKPulseStatus = uint32_t# RKTypes.h: 595

enum_anon_85 = c_int# RKTypes.h: 596

RKPulseStatusNull = 0# RKTypes.h: 596

RKPulseStatusVacant = 0# RKTypes.h: 596

RKPulseStatusHasIQData = 1# RKTypes.h: 596

RKPulseStatusHasPosition = (1 << 1)# RKTypes.h: 596

RKPulseStatusInspected = (1 << 2)# RKTypes.h: 596

RKPulseStatusCompressed = (1 << 3)# RKTypes.h: 596

RKPulseStatusSkipped = (1 << 4)# RKTypes.h: 596

RKPulseStatusDownSampled = (1 << 5)# RKTypes.h: 596

RKPulseStatusProcessed = (1 << 6)# RKTypes.h: 596

RKPulseStatusRingInspected = (1 << 7)# RKTypes.h: 596

RKPulseStatusRingFiltered = (1 << 8)# RKTypes.h: 596

RKPulseStatusRingSkipped = (1 << 9)# RKTypes.h: 596

RKPulseStatusRingProcessed = (1 << 10)# RKTypes.h: 596

RKPulseStatusReadyForMomentEngine = (((RKPulseStatusRingProcessed | RKPulseStatusProcessed) | RKPulseStatusHasPosition) | RKPulseStatusHasIQData)# RKTypes.h: 596

RKPulseStatusCompleteForMoments = ((RKPulseStatusReadyForMomentEngine | RKPulseStatusDownSampled) | RKPulseStatusCompressed)# RKTypes.h: 596

RKPulseStatusUsedForMoments = (1 << 11)# RKTypes.h: 596

RKPulseStatusProcessMask = (((((((((RKPulseStatusInspected | RKPulseStatusCompressed) | RKPulseStatusSkipped) | RKPulseStatusDownSampled) | RKPulseStatusProcessed) | RKPulseStatusRingInspected) | RKPulseStatusRingFiltered) | RKPulseStatusRingSkipped) | RKPulseStatusRingProcessed) | RKPulseStatusUsedForMoments)# RKTypes.h: 596

RKPulseStatusRecorded = (1 << 12)# RKTypes.h: 596

RKPulseStatusStreamed = (1 << 13)# RKTypes.h: 596

RKPulseStatusConsumed = (1 << 14)# RKTypes.h: 596

RKRayStatus = uint32_t# RKTypes.h: 633

enum_anon_86 = c_int# RKTypes.h: 634

RKRayStatusVacant = 0# RKTypes.h: 634

RKRayStatusProcessing = 1# RKTypes.h: 634

RKRayStatusProcessed = (1 << 1)# RKTypes.h: 634

RKRayStatusSkipped = (1 << 2)# RKTypes.h: 634

RKRayStatusReady = (1 << 3)# RKTypes.h: 634

RKRayStatusStreamed = (1 << 4)# RKTypes.h: 634

RKRayStatusConsumed = (1 << 5)# RKTypes.h: 634

RKRayStatusOverviewed = (1 << 6)# RKTypes.h: 634

RKInitFlag = uint32_t# RKTypes.h: 645

enum_anon_87 = c_int# RKTypes.h: 646

RKInitFlagNone = 0# RKTypes.h: 646

RKInitFlagVerbose = 0x00000001# RKTypes.h: 646

RKInitFlagVeryVerbose = 0x00000002# RKTypes.h: 646

RKInitFlagVeryVeryVerbose = 0x00000004# RKTypes.h: 646

RKInitFlagShowClockOffset = 0x00000008# RKTypes.h: 646

RKInitFlagManuallyAssignCPU = 0x00000010# RKTypes.h: 646

RKInitFlagIgnoreGPS = 0x00000020# RKTypes.h: 646

RKInitFlagIgnoreHeading = 0x00000040# RKTypes.h: 646

RKInitFlagReserved4 = 0x00000080# RKTypes.h: 646

RKInitFlagAllocStatusBuffer = 0x00000100# RKTypes.h: 646

RKInitFlagAllocConfigBuffer = 0x00000200# RKTypes.h: 646

RKInitFlagAllocRawIQBuffer = 0x00000400# RKTypes.h: 646

RKInitFlagAllocPositionBuffer = 0x00000800# RKTypes.h: 646

RKInitFlagAllocMomentBuffer = 0x00001000# RKTypes.h: 646

RKInitFlagAllocHealthBuffer = 0x00002000# RKTypes.h: 646

RKInitFlagAllocHealthNodes = 0x00004000# RKTypes.h: 646

RKInitFlagReserved1 = 0x00008000# RKTypes.h: 646

RKInitFlagPulsePositionCombiner = 0x00010000# RKTypes.h: 646

RKInitFlagPositionSteerEngine = 0x00020000# RKTypes.h: 646

RKInitFlagSignalProcessor = 0x00040000# RKTypes.h: 646

RKInitFlagStartPulseEngine = 0x00100000# RKTypes.h: 646

RKInitFlagStartRingFilterEngine = 0x00200000# RKTypes.h: 646

RKInitFlagStartMomentEngine = 0x00400000# RKTypes.h: 646

RKInitFlagStartRawDataRecorder = 0x00800000# RKTypes.h: 646

RKInitFlagRelay = 0x00007703# RKTypes.h: 646

RKInitFlagIQPlayback = 0x00047701# RKTypes.h: 646

RKInitFlagAllocEverything = 0x00077F01# RKTypes.h: 646

RKInitFlagAllocEverythingQuiet = 0x00077F00# RKTypes.h: 646

RKMomentList = uint32_t# RKTypes.h: 679

enum_anon_88 = c_int# RKTypes.h: 680

RKMomentListNull = 0# RKTypes.h: 680

RKMomentListHm = 1# RKTypes.h: 680

RKMomentListHmi = 1# RKTypes.h: 680

RKMomentListHmq = (1 << 1)# RKTypes.h: 680

RKMomentListHR0 = (1 << 2)# RKTypes.h: 680

RKMomentListHR1 = (1 << 3)# RKTypes.h: 680

RKMomentListHR1i = (1 << 3)# RKTypes.h: 680

RKMomentListHR1q = (1 << 4)# RKTypes.h: 680

RKMomentListHR2 = (1 << 5)# RKTypes.h: 680

RKMomentListHR3 = (1 << 6)# RKTypes.h: 680

RKMomentListHR4 = (1 << 7)# RKTypes.h: 680

RKMomentListVm = (1 << 8)# RKTypes.h: 680

RKMomentListVmi = (1 << 8)# RKTypes.h: 680

RKMomentListVmq = (1 << 9)# RKTypes.h: 680

RKMomentListVR0 = (1 << 10)# RKTypes.h: 680

RKMomentListVR1 = (1 << 11)# RKTypes.h: 680

RKMomentListVR1i = (1 << 11)# RKTypes.h: 680

RKMomentListVR1q = (1 << 12)# RKTypes.h: 680

RKMomentListVR2 = (1 << 13)# RKTypes.h: 680

RKMomentListVR3 = (1 << 14)# RKTypes.h: 680

RKMomentListVR4 = (1 << 15)# RKTypes.h: 680

RKMomentListC0 = (1 << 16)# RKTypes.h: 680

RKMomentListC0i = (1 << 16)# RKTypes.h: 680

RKMomentListC0q = (1 << 17)# RKTypes.h: 680

RKMomentListCn1 = (1 << 18)# RKTypes.h: 680

RKMomentListCp1 = (1 << 19)# RKTypes.h: 680

RKMomentListCn2 = (1 << 20)# RKTypes.h: 680

RKMomentListCp2 = (1 << 21)# RKTypes.h: 680

RKMomentListCn3 = (1 << 22)# RKTypes.h: 680

RKMomentListCp3 = (1 << 23)# RKTypes.h: 680

RKMomentListCn4 = (1 << 24)# RKTypes.h: 680

RKMomentListCp4 = (1 << 25)# RKTypes.h: 680

RKMomentListCa0 = (1 << 26)# RKTypes.h: 680

RKMomentListCb0 = (1 << 27)# RKTypes.h: 680

RKMomentListChcvx0 = (1 << 28)# RKTypes.h: 680

RKMomentListCvchx0 = (1 << 29)# RKTypes.h: 680

RKMomentIndex = uint8_t# RKTypes.h: 719

enum_anon_89 = c_int# RKTypes.h: 720

RKMomentIndexHmi = 0# RKTypes.h: 720

RKMomentIndexHmq = (RKMomentIndexHmi + 1)# RKTypes.h: 720

RKMomentIndexHR0 = (RKMomentIndexHmq + 1)# RKTypes.h: 720

RKMomentIndexHR1i = (RKMomentIndexHR0 + 1)# RKTypes.h: 720

RKMomentIndexHR1q = (RKMomentIndexHR1i + 1)# RKTypes.h: 720

RKMomentIndexHR2 = (RKMomentIndexHR1q + 1)# RKTypes.h: 720

RKMomentIndexHR3 = (RKMomentIndexHR2 + 1)# RKTypes.h: 720

RKMomentIndexHR4 = (RKMomentIndexHR3 + 1)# RKTypes.h: 720

RKMomentIndexVmi = (RKMomentIndexHR4 + 1)# RKTypes.h: 720

RKMomentIndexVmq = (RKMomentIndexVmi + 1)# RKTypes.h: 720

RKMomentIndexVR0 = (RKMomentIndexVmq + 1)# RKTypes.h: 720

RKMomentIndexVR1i = (RKMomentIndexVR0 + 1)# RKTypes.h: 720

RKMomentIndexVR1q = (RKMomentIndexVR1i + 1)# RKTypes.h: 720

RKMomentIndexVR2 = (RKMomentIndexVR1q + 1)# RKTypes.h: 720

RKMomentIndexVR3 = (RKMomentIndexVR2 + 1)# RKTypes.h: 720

RKMomentIndexVR4 = (RKMomentIndexVR3 + 1)# RKTypes.h: 720

RKmomentIndexC0i = (RKMomentIndexVR4 + 1)# RKTypes.h: 720

RKmomentIndexC0q = (RKmomentIndexC0i + 1)# RKTypes.h: 720

RKmomentIndexCn1 = (RKmomentIndexC0q + 1)# RKTypes.h: 720

RKmomentIndexCp1 = (RKmomentIndexCn1 + 1)# RKTypes.h: 720

RKmomentIndexCn2 = (RKmomentIndexCp1 + 1)# RKTypes.h: 720

RKmomentIndexCp2 = (RKmomentIndexCn2 + 1)# RKTypes.h: 720

RKmomentIndexCn3 = (RKmomentIndexCp2 + 1)# RKTypes.h: 720

RKmomentIndexCp3 = (RKmomentIndexCn3 + 1)# RKTypes.h: 720

RKmomentIndexCn4 = (RKmomentIndexCp3 + 1)# RKTypes.h: 720

RKmomentIndexCp4 = (RKmomentIndexCn4 + 1)# RKTypes.h: 720

RKMomentIndexCount = (RKmomentIndexCp4 + 1)# RKTypes.h: 720

RKProductList = uint32_t# RKTypes.h: 752

enum_anon_90 = c_int# RKTypes.h: 753

RKProductListNone = 0# RKTypes.h: 753

RKProductListUInt8Z = 1# RKTypes.h: 753

RKProductListUInt8V = (1 << 1)# RKTypes.h: 753

RKProductListUInt8W = (1 << 2)# RKTypes.h: 753

RKProductListUInt8D = (1 << 3)# RKTypes.h: 753

RKProductListUInt8P = (1 << 4)# RKTypes.h: 753

RKProductListUInt8R = (1 << 5)# RKTypes.h: 753

RKProductListUInt8K = (1 << 6)# RKTypes.h: 753

RKProductListUInt8Sh = (1 << 7)# RKTypes.h: 753

RKProductListUInt8Sv = (1 << 8)# RKTypes.h: 753

RKProductListUInt8Q = (1 << 9)# RKTypes.h: 753

RKProductListUInt8U6 = (1 << 10)# RKTypes.h: 753

RKProductListUInt8U5 = (1 << 11)# RKTypes.h: 753

RKProductListUInt8U4 = (1 << 12)# RKTypes.h: 753

RKProductListUInt8U3 = (1 << 13)# RKTypes.h: 753

RKProductListUInt8U2 = (1 << 14)# RKTypes.h: 753

RKProductListUInt8U1 = (1 << 15)# RKTypes.h: 753

RKProductListUInt8ZVWDPR = 0x0000003F# RKTypes.h: 753

RKProductListUInt8ZVWDPRK = 0x0000007F# RKTypes.h: 753

RKProductListUInt8ZVWDPRKS = 0x000001FF# RKTypes.h: 753

RKProductListUInt8ZVWDPRKSQ = 0x000003FF# RKTypes.h: 753

RKProductListUInt8All = 0x0000FFFF# RKTypes.h: 753

RKProductListFloatZ = (1 << 16)# RKTypes.h: 753

RKProductListFloatV = (1 << 17)# RKTypes.h: 753

RKProductListFloatW = (1 << 18)# RKTypes.h: 753

RKProductListFloatD = (1 << 19)# RKTypes.h: 753

RKProductListFloatP = (1 << 20)# RKTypes.h: 753

RKProductListFloatR = (1 << 21)# RKTypes.h: 753

RKProductListFloatK = (1 << 22)# RKTypes.h: 753

RKProductListFloatSh = (1 << 23)# RKTypes.h: 753

RKProductListFloatSv = (1 << 24)# RKTypes.h: 753

RKProductListFloatQ = (1 << 25)# RKTypes.h: 753

RKProductListFloatLh = (1 << 26)# RKTypes.h: 753

RKProductListFloatLv = (1 << 27)# RKTypes.h: 753

RKProductListFloatPXh = (1 << 28)# RKTypes.h: 753

RKProductListFloatPXv = (1 << 29)# RKTypes.h: 753

RKProductListFloatRXh = (1 << 30)# RKTypes.h: 753

RKProductListFloatRXv = (1 << 31)# RKTypes.h: 753

RKProductListFloatZVWDPR = 0x003F0000# RKTypes.h: 753

RKProductListFloatZVWDPRK = 0x007F0000# RKTypes.h: 753

RKProductListFloatZVWDPRKS = 0x01FF0000# RKTypes.h: 753

RKProductListFloatZVWDPRKLRXPX = 0xFC7F0000# RKTypes.h: 753

RKProductListFloatATSR = 0xFDFF0000# RKTypes.h: 753

RKProductListFloatZVWDPRKSQ = 0x03FF0000# RKTypes.h: 753

RKProductListFloatAll = 0xFFFF0000# RKTypes.h: 753

RKProductIndex = uint8_t# RKTypes.h: 801

enum_anon_91 = c_int# RKTypes.h: 802

RKProductIndexZ = 0# RKTypes.h: 802

RKProductIndexV = (RKProductIndexZ + 1)# RKTypes.h: 802

RKProductIndexW = (RKProductIndexV + 1)# RKTypes.h: 802

RKProductIndexD = (RKProductIndexW + 1)# RKTypes.h: 802

RKProductIndexP = (RKProductIndexD + 1)# RKTypes.h: 802

RKProductIndexR = (RKProductIndexP + 1)# RKTypes.h: 802

RKProductIndexK = (RKProductIndexR + 1)# RKTypes.h: 802

RKProductIndexSh = (RKProductIndexK + 1)# RKTypes.h: 802

RKProductIndexSv = (RKProductIndexSh + 1)# RKTypes.h: 802

RKProductIndexQ = (RKProductIndexSv + 1)# RKTypes.h: 802

RKProductIndexLh = (RKProductIndexQ + 1)# RKTypes.h: 802

RKProductIndexLv = (RKProductIndexLh + 1)# RKTypes.h: 802

RKProductIndexPXh = (RKProductIndexLv + 1)# RKTypes.h: 802

RKProductIndexPXv = (RKProductIndexPXh + 1)# RKTypes.h: 802

RKProductIndexRXh = (RKProductIndexPXv + 1)# RKTypes.h: 802

RKProductIndexRXv = (RKProductIndexRXh + 1)# RKTypes.h: 802

RKProductIndexZv = (RKProductIndexRXv + 1)# RKTypes.h: 802

RKProductIndexVv = (RKProductIndexZv + 1)# RKTypes.h: 802

RKProductIndexWv = (RKProductIndexVv + 1)# RKTypes.h: 802

RKProductIndexCount = (RKProductIndexWv + 1)# RKTypes.h: 802

RKProductType = uint8_t# RKTypes.h: 825

enum_anon_92 = c_int# RKTypes.h: 826

RKProductTypeUnknown = 0# RKTypes.h: 826

RKProductTypeCellMatch = 1# RKTypes.h: 826

RKProductTypePPI = (1 << 1)# RKTypes.h: 826

RKProductTypeCAPPI = (1 << 2)# RKTypes.h: 826

RKConfigKey = uint32_t# RKTypes.h: 833

enum_anon_93 = c_int# RKTypes.h: 834

RKConfigKeyNull = 0# RKTypes.h: 834

RKConfigKeyVolumeIndex = (RKConfigKeyNull + 1)# RKTypes.h: 834

RKConfigKeySweepIndex = (RKConfigKeyVolumeIndex + 1)# RKTypes.h: 834

RKConfigKeySweepElevation = (RKConfigKeySweepIndex + 1)# RKTypes.h: 834

RKConfigKeySweepAzimuth = (RKConfigKeySweepElevation + 1)# RKTypes.h: 834

RKConfigKeyPositionMarker = (RKConfigKeySweepAzimuth + 1)# RKTypes.h: 834

RKConfigKeyPRT = (RKConfigKeyPositionMarker + 1)# RKTypes.h: 834

RKConfigKeyPRF = (RKConfigKeyPRT + 1)# RKTypes.h: 834

RKConfigKeyDualPRF = (RKConfigKeyPRF + 1)# RKTypes.h: 834

RKConfigKeyPulseGateCount = (RKConfigKeyDualPRF + 1)# RKTypes.h: 834

RKConfigKeyPulseGateSize = (RKConfigKeyPulseGateCount + 1)# RKTypes.h: 834

RKConfigKeyPulseWidth = (RKConfigKeyPulseGateSize + 1)# RKTypes.h: 834

RKConfigKeyWaveform = (RKConfigKeyPulseWidth + 1)# RKTypes.h: 834

RKConfigKeyWaveformDecimate = (RKConfigKeyWaveform + 1)# RKTypes.h: 834

RKConfigKeyWaveformId = (RKConfigKeyWaveformDecimate + 1)# RKTypes.h: 834

RKConfigKeyWaveformName = (RKConfigKeyWaveformId + 1)# RKTypes.h: 834

RKConfigKeySystemNoise = (RKConfigKeyWaveformName + 1)# RKTypes.h: 834

RKConfigKeySystemZCal = (RKConfigKeySystemNoise + 1)# RKTypes.h: 834

RKConfigKeySystemDCal = (RKConfigKeySystemZCal + 1)# RKTypes.h: 834

RKConfigKeySystemPCal = (RKConfigKeySystemDCal + 1)# RKTypes.h: 834

RKConfigKeyWaveformCalibration = (RKConfigKeySystemPCal + 1)# RKTypes.h: 834

RKConfigKeySNRThreshold = (RKConfigKeyWaveformCalibration + 1)# RKTypes.h: 834

RKConfigKeySQIThreshold = (RKConfigKeySNRThreshold + 1)# RKTypes.h: 834

RKConfigKeyVCPDefinition = (RKConfigKeySQIThreshold + 1)# RKTypes.h: 834

RKConfigKeyRingFilterGateCount = (RKConfigKeyVCPDefinition + 1)# RKTypes.h: 834

RKConfigKeyTransitionGateCount = (RKConfigKeyRingFilterGateCount + 1)# RKTypes.h: 834

RKConfigKeyUserIntegerParameters = (RKConfigKeyTransitionGateCount + 1)# RKTypes.h: 834

RKConfigKeyUserFloatParameters = (RKConfigKeyUserIntegerParameters + 1)# RKTypes.h: 834

RKConfigKeyUserResource = (RKConfigKeyUserFloatParameters + 1)# RKTypes.h: 834

RKConfigKeyMomentMethod = (RKConfigKeyUserResource + 1)# RKTypes.h: 834

RKConfigKeyCount = (RKConfigKeyMomentMethod + 1)# RKTypes.h: 834

RKHealthNode = uint8_t# RKTypes.h: 868

enum_anon_94 = c_int# RKTypes.h: 869

RKHealthNodeRadarKit = 0# RKTypes.h: 869

RKHealthNodeTransceiver = (RKHealthNodeRadarKit + 1)# RKTypes.h: 869

RKHealthNodePedestal = (RKHealthNodeTransceiver + 1)# RKTypes.h: 869

RKHealthNodeTweeta = (RKHealthNodePedestal + 1)# RKTypes.h: 869

RKHealthNodeUser1 = (RKHealthNodeTweeta + 1)# RKTypes.h: 869

RKHealthNodeUser2 = (RKHealthNodeUser1 + 1)# RKTypes.h: 869

RKHealthNodeUser3 = (RKHealthNodeUser2 + 1)# RKTypes.h: 869

RKHealthNodeUser4 = (RKHealthNodeUser3 + 1)# RKTypes.h: 869

RKHealthNodeUser5 = (RKHealthNodeUser4 + 1)# RKTypes.h: 869

RKHealthNodeUser6 = (RKHealthNodeUser5 + 1)# RKTypes.h: 869

RKHealthNodeUser7 = (RKHealthNodeUser6 + 1)# RKTypes.h: 869

RKHealthNodeUser8 = (RKHealthNodeUser7 + 1)# RKTypes.h: 869

RKHealthNodeCount = (RKHealthNodeUser8 + 1)# RKTypes.h: 869

RKHealthNodeInvalid = (RKHealthNode (ord_if_char((-1)))).value# RKTypes.h: 869

RKScriptProperty = uint8_t# RKTypes.h: 886

enum_anon_95 = c_int# RKTypes.h: 887

RKScriptPropertyNull = 0# RKTypes.h: 887

RKScriptPropertyProduceZip = 1# RKTypes.h: 887

RKScriptPropertyProduceTgz = (1 << 1)# RKTypes.h: 887

RKScriptPropertyProduceTarXz = (1 << 2)# RKTypes.h: 887

RKScriptPropertyProduceTxz = (1 << 3)# RKTypes.h: 887

RKScriptPropertyProduceArchive = (((RKScriptPropertyProduceZip | RKScriptPropertyProduceTgz) | RKScriptPropertyProduceTarXz) | RKScriptPropertyProduceTxz)# RKTypes.h: 887

RKScriptPropertyRemoveNCFiles = (1 << 4)# RKTypes.h: 887

RKEngineState = uint32_t# RKTypes.h: 923

enum_anon_96 = c_int# RKTypes.h: 924

RKEngineStateNull = 0# RKTypes.h: 924

RKEngineStateSleep0 = 1# RKTypes.h: 924

RKEngineStateSleep1 = (1 << 1)# RKTypes.h: 924

RKEngineStateSleep2 = (1 << 2)# RKTypes.h: 924

RKEngineStateSleep3 = (1 << 3)# RKTypes.h: 924

RKEngineStateSleepMask = 0x0000000F# RKTypes.h: 924

RKEngineStateWritingFile = (1 << 4)# RKTypes.h: 924

RKEngineStateMemoryChange = (1 << 5)# RKTypes.h: 924

RKEngineStateSuspended = (1 << 6)# RKTypes.h: 924

RKEngineStateBusyMask = 0x000000F0# RKTypes.h: 924

RKEngineStateReserved = (1 << 7)# RKTypes.h: 924

RKEngineStateAllocated = (1 << 8)# RKTypes.h: 924

RKEngineStateProperlyWired = (1 << 9)# RKTypes.h: 924

RKEngineStateActivating = (1 << 10)# RKTypes.h: 924

RKEngineStateDeactivating = (1 << 11)# RKTypes.h: 924

RKEngineStateActive = (1 << 13)# RKTypes.h: 924

RKEngineStateWantActive = (1 << 15)# RKTypes.h: 924

RKEngineStateMainMask = 0x0000FF00# RKTypes.h: 924

RKEngineStateChildAllocated = (1 << 16)# RKTypes.h: 924

RKEngineStateChildProperlyWired = (1 << 17)# RKTypes.h: 924

RKEngineStateChildActivating = (1 << 18)# RKTypes.h: 924

RKEngineStateChildDeactivating = (1 << 19)# RKTypes.h: 924

RKEngineStateChildActive = (1 << 20)# RKTypes.h: 924

RKEngineStateChildMask = 0x001F0000# RKTypes.h: 924

RKStatusEnum = uint32_t# RKTypes.h: 951

enum_anon_97 = c_int# RKTypes.h: 952

RKStatusEnumUnknown = (-3)# RKTypes.h: 952

RKStatusEnumOld = (-3)# RKTypes.h: 952

RKStatusEnumInvalid = (-2)# RKTypes.h: 952

RKStatusEnumTooLow = (-2)# RKTypes.h: 952

RKStatusEnumLow = (-1)# RKTypes.h: 952

RKStatusEnumNormal = 0# RKTypes.h: 952

RKStatusEnumActive = 0# RKTypes.h: 952

RKStatusEnumHigh = 1# RKTypes.h: 952

RKStatusEnumStandby = 1# RKTypes.h: 952

RKStatusEnumInactive = 1# RKTypes.h: 952

RKStatusEnumOutOfRange = 1# RKTypes.h: 952

RKStatusEnumTooHigh = 2# RKTypes.h: 952

RKStatusEnumNotOperational = 2# RKTypes.h: 952

RKStatusEnumOff = 2# RKTypes.h: 952

RKStatusEnumFault = 2# RKTypes.h: 952

RKStatusEnumNotWired = 3# RKTypes.h: 952

RKStatusEnumCritical = 4# RKTypes.h: 952

RKFileType = uint32_t# RKTypes.h: 972

enum_anon_98 = c_int# RKTypes.h: 973

RKFileTypeIQ = 0# RKTypes.h: 973

RKFileTypeMoment = (RKFileTypeIQ + 1)# RKTypes.h: 973

RKFileTypeHealth = (RKFileTypeMoment + 1)# RKTypes.h: 973

RKFileTypeLog = (RKFileTypeHealth + 1)# RKTypes.h: 973

RKFileTypeCount = (RKFileTypeLog + 1)# RKTypes.h: 973

RKStream = uint64_t# RKTypes.h: 981

enum_anon_99 = c_int# RKTypes.h: 982

RKStreamNull = 0# RKTypes.h: 982

RKStreamStatusMask = 0x0F# RKTypes.h: 982

RKStreamStatusPositions = 1# RKTypes.h: 982

RKStreamStatusPulses = 2# RKTypes.h: 982

RKStreamStatusRays = 3# RKTypes.h: 982

RKStreamStatusIngest = 4# RKTypes.h: 982

RKStreamStatusEngines = 5# RKTypes.h: 982

RKStreamStatusBuffers = 6# RKTypes.h: 982

RKStreamASCIIArtZ = 7# RKTypes.h: 982

RKStreamASCIIArtHealth = 8# RKTypes.h: 982

RKStreamASCIIArtVCP = 9# RKTypes.h: 982

RKStreamStatusAll = 0xFF# RKTypes.h: 982

RKStreamHealthInJSON = (1 << 5)# RKTypes.h: 982

RKStreamStatusEngineBinary = (1 << 6)# RKTypes.h: 982

RKStreamStatusProcessorStatus = (1 << 7)# RKTypes.h: 982

RKStreamDisplayIQ = (1 << 8)# RKTypes.h: 982

RKStreamDisplayIQFiltered = (1 << 9)# RKTypes.h: 982

RKStreamProductIQ = (1 << 10)# RKTypes.h: 982

RKStreamProductIQFiltered = (1 << 11)# RKTypes.h: 982

RKStreamControl = (1 << 15)# RKTypes.h: 982

RKStreamScopeStuff = 0x0000000000000300# RKTypes.h: 982

RKStreamDisplayZ = (1 << 16)# RKTypes.h: 982

RKStreamDisplayV = (1 << 17)# RKTypes.h: 982

RKStreamDisplayW = (1 << 18)# RKTypes.h: 982

RKStreamDisplayD = (1 << 19)# RKTypes.h: 982

RKStreamDisplayP = (1 << 20)# RKTypes.h: 982

RKStreamDisplayR = (1 << 21)# RKTypes.h: 982

RKStreamDisplayK = (1 << 22)# RKTypes.h: 982

RKStreamDisplaySh = (1 << 23)# RKTypes.h: 982

RKStreamDisplaySv = (1 << 24)# RKTypes.h: 982

RKStreamDisplayQ = (1 << 25)# RKTypes.h: 982

RKStreamDisplayZVWDPRKS = 0x0000000001FF0000# RKTypes.h: 982

RKStreamDisplayAll = 0x0000000003FF0000# RKTypes.h: 982

RKStreamProductZ = (1 << 32)# RKTypes.h: 982

RKStreamProductV = (1 << 33)# RKTypes.h: 982

RKStreamProductW = (1 << 34)# RKTypes.h: 982

RKStreamProductD = (1 << 35)# RKTypes.h: 982

RKStreamProductP = (1 << 36)# RKTypes.h: 982

RKStreamProductR = (1 << 37)# RKTypes.h: 982

RKStreamProductK = (1 << 38)# RKTypes.h: 982

RKStreamProductSh = (1 << 39)# RKTypes.h: 982

RKStreamProductSv = (1 << 40)# RKTypes.h: 982

RKStreamProductQ = (1 << 41)# RKTypes.h: 982

RKStreamProductZVWDPRKS = 0x000001FF00000000# RKTypes.h: 982

RKStreamProductAll = 0x000003FF00000000# RKTypes.h: 982

RKStreamSweepZ = (1 << 48)# RKTypes.h: 982

RKStreamSweepV = (1 << 49)# RKTypes.h: 982

RKStreamSweepW = (1 << 50)# RKTypes.h: 982

RKStreamSweepD = (1 << 51)# RKTypes.h: 982

RKStreamSweepP = (1 << 52)# RKTypes.h: 982

RKStreamSweepR = (1 << 53)# RKTypes.h: 982

RKStreamSweepK = (1 << 54)# RKTypes.h: 982

RKStreamSweepSh = (1 << 55)# RKTypes.h: 982

RKStreamSweepSv = (1 << 56)# RKTypes.h: 982

RKStreamSweepQ = (1 << 57)# RKTypes.h: 982

RKStreamSweepZVWDPRKS = 0x01FF000000000000# RKTypes.h: 982

RKStreamSweepAll = 0x03FF000000000000# RKTypes.h: 982

RKStreamAlmostEverything = 0x03FF03FF03FFF000# RKTypes.h: 982

RKStreamStatusTerminalChange = 0x0400000000000000# RKTypes.h: 982

RKHostStatus = uint8_t# RKTypes.h: 1044

enum_anon_100 = c_int# RKTypes.h: 1045

RKHostStatusUnknown = 0# RKTypes.h: 1045

RKHostStatusUnreachable = (RKHostStatusUnknown + 1)# RKTypes.h: 1045

RKHostStatusPartiallyReachable = (RKHostStatusUnreachable + 1)# RKTypes.h: 1045

RKHostStatusReachableUnusual = (RKHostStatusPartiallyReachable + 1)# RKTypes.h: 1045

RKHostStatusReachable = (RKHostStatusReachableUnusual + 1)# RKTypes.h: 1045

RKProductStatus = uint32_t# RKTypes.h: 1053

enum_anon_101 = c_int# RKTypes.h: 1054

RKProductStatusVacant = 0# RKTypes.h: 1054

RKProductStatusActive = (1 << 0)# RKTypes.h: 1054

RKProductStatusBusy = (1 << 1)# RKTypes.h: 1054

RKProductStatusSkipped = (1 << 2)# RKTypes.h: 1054

RKProductStatusSleep0 = (1 << 4)# RKTypes.h: 1054

RKProductStatusSleep1 = (1 << 5)# RKTypes.h: 1054

RKProductStatusSleep2 = (1 << 6)# RKTypes.h: 1054

RKProductStatusSleep3 = (1 << 7)# RKTypes.h: 1054

RKTextPreferences = uint32_t# RKTypes.h: 1065

enum_anon_102 = c_int# RKTypes.h: 1066

RKTextPreferencesNone = 0# RKTypes.h: 1066

RKTextPreferencesShowColor = 1# RKTypes.h: 1066

RKTextPreferencesDrawBackground = (1 << 1)# RKTypes.h: 1066

RKTextPreferencesWindowSizeMask = (7 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize80x25 = (0 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize80x40 = (1 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize80x50 = (2 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize120x40 = (3 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize120x50 = (4 << 2)# RKTypes.h: 1066

RKTextPreferencesWindowSize120x80 = (5 << 2)# RKTypes.h: 1066

RKTextPreferencesShowDebuggingMessage = (1 << 7)# RKTypes.h: 1066

RKWaveformType = uint32_t# RKTypes.h: 1080

enum_anon_103 = c_int# RKTypes.h: 1081

RKWaveformTypeNone = 0# RKTypes.h: 1081

RKWaveformTypeIsComplex = 1# RKTypes.h: 1081

RKWaveformTypeSingleTone = (1 << 1)# RKTypes.h: 1081

RKWaveformTypeFrequencyHopping = (1 << 2)# RKTypes.h: 1081

RKWaveformTypeLinearFrequencyModulation = (1 << 3)# RKTypes.h: 1081

RKWaveformTypeTimeFrequencyMultiplexing = (1 << 4)# RKTypes.h: 1081

RKWaveformTypeFromFile = (1 << 5)# RKTypes.h: 1081

RKWaveformTypeFlatAnchors = (1 << 6)# RKTypes.h: 1081

RKWaveformTypeFrequencyHoppingChirp = (1 << 7)# RKTypes.h: 1081

RKEventType = uint32_t# RKTypes.h: 1093

enum_anon_104 = c_int# RKTypes.h: 1094

RKEventTypeNull = 0# RKTypes.h: 1094

RKEventTypeRaySweepBegin = (RKEventTypeNull + 1)# RKTypes.h: 1094

RKEventTypeRaySweepEnd = (RKEventTypeRaySweepBegin + 1)# RKTypes.h: 1094

RKFilterType = uint8_t# RKTypes.h: 1100

enum_anon_105 = c_int# RKTypes.h: 1101

RKFilterTypeNull = 0# RKTypes.h: 1101

RKFilterTypeElliptical1 = (RKFilterTypeNull + 1)# RKTypes.h: 1101

RKFilterTypeElliptical2 = (RKFilterTypeElliptical1 + 1)# RKTypes.h: 1101

RKFilterTypeElliptical3 = (RKFilterTypeElliptical2 + 1)# RKTypes.h: 1101

RKFilterTypeElliptical4 = (RKFilterTypeElliptical3 + 1)# RKTypes.h: 1101

RKFilterTypeImpulse = (RKFilterTypeElliptical4 + 1)# RKTypes.h: 1101

RKFilterTypeCount = (RKFilterTypeImpulse + 1)# RKTypes.h: 1101

RKFilterTypeUserDefined = (RKFilterTypeCount + 1)# RKTypes.h: 1101

RKFilterTypeTest1 = (RKFilterTypeUserDefined + 1)# RKTypes.h: 1101

RKPedestalInstructType = c_int# RKTypes.h: 1113

enum_anon_106 = c_int# RKTypes.h: 1114

RKPedestalInstructTypeNone = 0# RKTypes.h: 1114

RKPedestalInstructTypeModeStandby = 1# RKTypes.h: 1114

RKPedestalInstructTypeModeEnable = 2# RKTypes.h: 1114

RKPedestalInstructTypeModeDisable = 3# RKTypes.h: 1114

RKPedestalInstructTypeModeSlew = 4# RKTypes.h: 1114

RKPedestalInstructTypeModePoint = 5# RKTypes.h: 1114

RKPedestalInstructTypeModeReset = 6# RKTypes.h: 1114

RKPedestalInstructTypeModeTest = 7# RKTypes.h: 1114

RKPedestalInstructTypeModeMask = 0x0F# RKTypes.h: 1114

RKPedestalInstructTypeAxisElevation = 0x10# RKTypes.h: 1114

RKPedestalInstructTypeAxisAzimuth = 0x20# RKTypes.h: 1114

RKPedestalInstructTypeAxisMask = 0x30# RKTypes.h: 1114

RKRawDataType = uint8_t# RKTypes.h: 1129

enum_anon_107 = c_int# RKTypes.h: 1130

RKRawDataTypeNull = 0# RKTypes.h: 1130

RKRawDataTypeFromTransceiver = (RKRawDataTypeNull + 1)# RKTypes.h: 1130

RKRawDataTypeAfterMatchedFilter = (RKRawDataTypeFromTransceiver + 1)# RKTypes.h: 1130

RKCompressorOption = uint8_t# RKTypes.h: 1136

enum_anon_108 = c_int# RKTypes.h: 1137

RKCompressorOptionRKInt16C = 0# RKTypes.h: 1137

RKCompressorOptionRKComplex = 1# RKTypes.h: 1137

RKCompressorOptionSingleChannel = (1 << 5)# RKTypes.h: 1137

RadarHubType = uint8_t# RKTypes.h: 1143

enum_anon_109 = c_int# RKTypes.h: 1144

RKRadarHubTypeHandshake = 1# RKTypes.h: 1144

RKRadarHubTypeControl = 2# RKTypes.h: 1144

RKRadarHubTypeHealth = 3# RKTypes.h: 1144

RKRadarHubTypeReserve4 = 4# RKTypes.h: 1144

RKRadarHubTypeScope = 5# RKTypes.h: 1144

RKRadarHubTypeResponse = 6# RKTypes.h: 1144

RKRadarHubTypeReserved7 = 7# RKTypes.h: 1144

RKRadarHubTypeReserved8 = 8# RKTypes.h: 1144

RKRadarHubTypeReserved9 = 9# RKTypes.h: 1144

RKRadarHubTypeReserved10 = 10# RKTypes.h: 1144

RKRadarHubTypeReserved11 = 11# RKTypes.h: 1144

RKRadarHubTypeReserved12 = 12# RKTypes.h: 1144

RKRadarHubTypeReserved13 = 13# RKTypes.h: 1144

RKRadarHubTypeReserved14 = 14# RKTypes.h: 1144

RKRadarHubTypeReserved15 = 15# RKTypes.h: 1144

RKRadarHubTypeRadialZ = 16# RKTypes.h: 1144

RKRadarHubTypeRadialV = 17# RKTypes.h: 1144

RKRadarHubTypeRadialW = 18# RKTypes.h: 1144

RKRadarHubTypeRadialD = 19# RKTypes.h: 1144

RKRadarHubTypeRadialP = 20# RKTypes.h: 1144

RKRadarHubTypeRadialR = 21# RKTypes.h: 1144

RKWriterOption = uint32_t# RKTypes.h: 1168

enum_RKWriterOption = c_int# RKTypes.h: 1169

RKWriterOptionNone = 0# RKTypes.h: 1169

RKWriterOptionPackPosition = 1# RKTypes.h: 1169

RKWriterOptionDeflateFields = (1 << 1)# RKTypes.h: 1169

RKWriterOptionStringVariables = (1 << 2)# RKTypes.h: 1169

RKMomentMethod = uint8_t# RKTypes.h: 1176

enum_RKMomentMethod = c_int# RKTypes.h: 1177

RKMomentMethodNone = 0# RKTypes.h: 1177

RKMomentMethodPulsePair = (RKMomentMethodNone + 1)# RKTypes.h: 1177

RKMomentMethodPulsePairHop = (RKMomentMethodPulsePair + 1)# RKTypes.h: 1177

RKMomentMethodPulsePairATSR = (RKMomentMethodPulsePairHop + 1)# RKTypes.h: 1177

RKMomentMethodMultiLag2 = (RKMomentMethodPulsePairATSR + 1)# RKTypes.h: 1177

RKMomentMethodMultiLag3 = (RKMomentMethodMultiLag2 + 1)# RKTypes.h: 1177

RKMomentMethodMultiLag4 = (RKMomentMethodMultiLag3 + 1)# RKTypes.h: 1177

RKMomentMethodSpectralMoment = (RKMomentMethodMultiLag4 + 1)# RKTypes.h: 1177

RKMomentMethodUserDefined = (RKMomentMethodSpectralMoment + 1)# RKTypes.h: 1177

# RKTypes.h: 1190
class struct_anon_110(Structure):
    pass

struct_anon_110._pack_ = 1
struct_anon_110.__slots__ = [
    'type',
    'counter',
    'startElevation',
    'endElevation',
    'startAzimuth',
    'endAzimuth',
    'rangeStart',
    'rangeDelta',
    'gateCount',
]
struct_anon_110._fields_ = [
    ('type', uint8_t),
    ('counter', uint8_t),
    ('startElevation', c_int16),
    ('endElevation', c_int16),
    ('startAzimuth', uint16_t),
    ('endAzimuth', uint16_t),
    ('rangeStart', uint16_t),
    ('rangeDelta', uint16_t),
    ('gateCount', uint16_t),
]

# RKTypes.h: 1202
class union_rk_radarhub_ray_header(Union):
    pass

union_rk_radarhub_ray_header._pack_ = 1
union_rk_radarhub_ray_header.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_radarhub_ray_header._anonymous_ = [
    'unnamed_1',
]
union_rk_radarhub_ray_header._fields_ = [
    ('unnamed_1', struct_anon_110),
    ('bytes', RKByte * int(16)),
]

RKRadarHubRayHeader = union_rk_radarhub_ray_header# RKTypes.h: 1202

# RKTypes.h: 1205
class struct_anon_111(Structure):
    pass

struct_anon_111._pack_ = 1
struct_anon_111.__slots__ = [
    'header',
    'data',
]
struct_anon_111._fields_ = [
    ('header', RKRadarHubRayHeader),
    ('data', RKByte * int(262144)),
]

# RKTypes.h: 1210
class union_rk_radarhub_ray(Union):
    pass

union_rk_radarhub_ray._pack_ = 1
union_rk_radarhub_ray.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_radarhub_ray._anonymous_ = [
    'unnamed_1',
]
union_rk_radarhub_ray._fields_ = [
    ('unnamed_1', struct_anon_111),
    ('bytes', POINTER(RKByte)),
]

RKRadarHubRay = union_rk_radarhub_ray# RKTypes.h: 1210

# RKTypes.h: 1255
class struct_rk_radar_desc(Structure):
    pass

struct_rk_radar_desc._pack_ = 1
struct_rk_radar_desc.__slots__ = [
    'initFlags',
    'pulseCapacity',
    'pulseToRayRatio',
    'doNotUse',
    'healthNodeCount',
    'healthBufferDepth',
    'statusBufferDepth',
    'configBufferDepth',
    'positionBufferDepth',
    'pulseBufferDepth',
    'rayBufferDepth',
    'productBufferDepth',
    'controlCapacity',
    'waveformCalibrationCapacity',
    'healthNodeBufferSize',
    'healthBufferSize',
    'statusBufferSize',
    'configBufferSize',
    'positionBufferSize',
    'pulseBufferSize',
    'rayBufferSize',
    'productBufferSize',
    'pulseSmoothFactor',
    'pulseTicsPerSecond',
    'positionSmoothFactor',
    'positionTicsPerSecond',
    'positionLatency',
    'latitude',
    'longitude',
    'heading',
    'radarHeight',
    'wavelength',
    'name',
    'filePrefix',
    'dataPath',
]
struct_rk_radar_desc._fields_ = [
    ('initFlags', RKInitFlag),
    ('pulseCapacity', uint32_t),
    ('pulseToRayRatio', uint16_t),
    ('doNotUse', uint16_t),
    ('healthNodeCount', uint32_t),
    ('healthBufferDepth', uint32_t),
    ('statusBufferDepth', uint32_t),
    ('configBufferDepth', uint32_t),
    ('positionBufferDepth', uint32_t),
    ('pulseBufferDepth', uint32_t),
    ('rayBufferDepth', uint32_t),
    ('productBufferDepth', uint32_t),
    ('controlCapacity', uint32_t),
    ('waveformCalibrationCapacity', uint32_t),
    ('healthNodeBufferSize', c_size_t),
    ('healthBufferSize', c_size_t),
    ('statusBufferSize', c_size_t),
    ('configBufferSize', c_size_t),
    ('positionBufferSize', c_size_t),
    ('pulseBufferSize', c_size_t),
    ('rayBufferSize', c_size_t),
    ('productBufferSize', c_size_t),
    ('pulseSmoothFactor', uint32_t),
    ('pulseTicsPerSecond', uint32_t),
    ('positionSmoothFactor', uint32_t),
    ('positionTicsPerSecond', uint32_t),
    ('positionLatency', c_double),
    ('latitude', c_double),
    ('longitude', c_double),
    ('heading', c_float),
    ('radarHeight', c_float),
    ('wavelength', c_float),
    ('name', RKName),
    ('filePrefix', c_char * int(8)),
    ('dataPath', c_char * int(768)),
]

RKRadarDesc = struct_rk_radar_desc# RKTypes.h: 1255

# RKTypes.h: 1268
class struct_rk_waveform(Structure):
    pass

struct_rk_waveform._pack_ = 1
struct_rk_waveform.__slots__ = [
    'name',
    'type',
    'count',
    'depth',
    'fc',
    'fs',
    'filterCounts',
    'filterAnchors',
    'samples',
    'iSamples',
]
struct_rk_waveform._fields_ = [
    ('name', RKName),
    ('type', RKWaveformType),
    ('count', uint8_t),
    ('depth', uint32_t),
    ('fc', c_double),
    ('fs', c_double),
    ('filterCounts', uint8_t * int(22)),
    ('filterAnchors', RKFilterAnchorGroup * int(22)),
    ('samples', POINTER(RKComplex) * int(22)),
    ('iSamples', POINTER(RKInt16C) * int(22)),
]

RKWaveform = struct_rk_waveform# RKTypes.h: 1268

# RKTypes.h: 1271
class struct_anon_112(Structure):
    pass

struct_anon_112._pack_ = 1
struct_anon_112.__slots__ = [
    'count',
    'depth',
    'type',
    'name',
    'fc',
    'fs',
    'filterCounts',
]
struct_anon_112._fields_ = [
    ('count', uint8_t),
    ('depth', uint32_t),
    ('type', RKWaveformType),
    ('name', RKName),
    ('fc', c_double),
    ('fs', c_double),
    ('filterCounts', uint8_t * int(22)),
]

# RKTypes.h: 1281
class union_rk_wave_file_header(Union):
    pass

union_rk_wave_file_header._pack_ = 1
union_rk_wave_file_header.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_wave_file_header._anonymous_ = [
    'unnamed_1',
]
union_rk_wave_file_header._fields_ = [
    ('unnamed_1', struct_anon_112),
    ('bytes', c_char * int(512)),
]

RKWaveFileGlobalHeader = union_rk_wave_file_header# RKTypes.h: 1281

# RKTypes.h: 1290
class struct_rk_waveform_cal(Structure):
    pass

struct_rk_waveform_cal._pack_ = 1
struct_rk_waveform_cal.__slots__ = [
    'uid',
    'name',
    'count',
    'ZCal',
    'DCal',
    'PCal',
]
struct_rk_waveform_cal._fields_ = [
    ('uid', uint32_t),
    ('name', RKName),
    ('count', uint8_t),
    ('ZCal', (RKFloat * int(2)) * int(8)),
    ('DCal', RKFloat * int(8)),
    ('PCal', RKFloat * int(8)),
]

RKWaveformCalibration = struct_rk_waveform_cal# RKTypes.h: 1290

# RKTypes.h: 1297
class struct_rk_waveform_response(Structure):
    pass

struct_rk_waveform_response._pack_ = 1
struct_rk_waveform_response.__slots__ = [
    'count',
    'length',
    'amplitudeResponse',
    'phaseResponse',
]
struct_rk_waveform_response._fields_ = [
    ('count', uint32_t),
    ('length', uint32_t),
    ('amplitudeResponse', POINTER(POINTER(RKFloat))),
    ('phaseResponse', POINTER(POINTER(RKFloat))),
]

RKWaveformResponse = struct_rk_waveform_response# RKTypes.h: 1297

# RKTypes.h: 1303
class struct_anon_113(Structure):
    pass

struct_anon_113._pack_ = 1
struct_anon_113.__slots__ = [
    'i',
    'volumeIndex',
    'sweepIndex',
    'sweepElevation',
    'sweepAzimuth',
    'startMarker',
    'prt',
    'pw',
    'pulseGateCount',
    'pulseGateSize',
    'transitionGateCount',
    'ringFilterGateCount',
    'waveformId',
    'noise',
    'systemZCal',
    'systemDCal',
    'systemPCal',
    'ZCal',
    'DCal',
    'PCal',
    'SNRThreshold',
    'SQIThreshold',
    'waveformName',
    'waveform',
    'waveformDecimate',
    'userResource',
    'momentMethod',
    'userIntegerParameters',
    'userFloatParameters',
    'vcpDefinition',
]
struct_anon_113._fields_ = [
    ('i', RKIdentifier),
    ('volumeIndex', RKIdentifier),
    ('sweepIndex', RKIdentifier),
    ('sweepElevation', c_float),
    ('sweepAzimuth', c_float),
    ('startMarker', RKMarker),
    ('prt', RKFloat * int(8)),
    ('pw', RKFloat * int(8)),
    ('pulseGateCount', uint32_t),
    ('pulseGateSize', RKFloat),
    ('transitionGateCount', uint32_t),
    ('ringFilterGateCount', uint32_t),
    ('waveformId', uint32_t * int(8)),
    ('noise', RKFloat * int(2)),
    ('systemZCal', RKFloat * int(2)),
    ('systemDCal', RKFloat),
    ('systemPCal', RKFloat),
    ('ZCal', (RKFloat * int(2)) * int(8)),
    ('DCal', RKFloat * int(8)),
    ('PCal', RKFloat * int(8)),
    ('SNRThreshold', RKFloat),
    ('SQIThreshold', RKFloat),
    ('waveformName', RKName),
    ('waveform', POINTER(RKWaveform)),
    ('waveformDecimate', POINTER(RKWaveform)),
    ('userResource', RKUserResource),
    ('momentMethod', RKMomentMethod),
    ('userIntegerParameters', uint32_t * int(8)),
    ('userFloatParameters', c_float * int(8)),
    ('vcpDefinition', c_char * int(480)),
]

# RKTypes.h: 1336
class union_rk_config(Union):
    pass

union_rk_config._pack_ = 1
union_rk_config.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_config._anonymous_ = [
    'unnamed_1',
]
union_rk_config._fields_ = [
    ('unnamed_1', struct_anon_113),
    ('bytes', RKByte * int(1024)),
]

RKConfig = union_rk_config# RKTypes.h: 1336

# RKTypes.h: 1342
class struct_anon_114(Structure):
    pass

struct_anon_114._pack_ = 1
struct_anon_114.__slots__ = [
    'i',
    'flag',
    'time',
    'timeDouble',
    'string',
]
struct_anon_114._fields_ = [
    ('i', RKIdentifier),
    ('flag', RKHealthFlag),
    ('time', struct_timeval),
    ('timeDouble', c_double),
    ('string', c_char * int(4096)),
]

# RKTypes.h: 1350
class union_rk_health(Union):
    pass

union_rk_health._pack_ = 1
union_rk_health.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_health._anonymous_ = [
    'unnamed_1',
]
union_rk_health._fields_ = [
    ('unnamed_1', struct_anon_114),
    ('bytes', POINTER(RKByte)),
]

RKHealth = union_rk_health# RKTypes.h: 1350

# RKTypes.h: 1359
class struct_rk_nodal_health(Structure):
    pass

struct_rk_nodal_health._pack_ = 1
struct_rk_nodal_health.__slots__ = [
    'healths',
    'index',
    'active',
]
struct_rk_nodal_health._fields_ = [
    ('healths', POINTER(RKHealth)),
    ('index', uint32_t),
    ('active', c_bool),
]

RKNodalHealth = struct_rk_nodal_health# RKTypes.h: 1359

# RKTypes.h: 1365
class struct_anon_115(Structure):
    pass

struct_anon_115._pack_ = 1
struct_anon_115.__slots__ = [
    'i',
    'tic',
    'rawElevation',
    'rawAzimuth',
    'rawElevationVelocity',
    'rawAzimuthVelocity',
    'rawElevationStatus',
    'rawAzimuthStatus',
    'queueSize',
    'elevationMode',
    'azimuthMode',
    'sequence',
    'flag',
    'elevationDegrees',
    'azimuthDegrees',
    'elevationVelocityDegreesPerSecond',
    'azimuthVelocityDegreesPerSecond',
    'reserved1',
    'reserved2',
    'reserved3',
    'reserved4',
    'sweepElevationDegrees',
    'sweepAzimuthDegrees',
    'time',
    'timeDouble',
    'volumeIndex',
    'sweepIndex',
]
struct_anon_115._fields_ = [
    ('i', RKIdentifier),
    ('tic', uint64_t),
    ('rawElevation', RKFourByte),
    ('rawAzimuth', RKFourByte),
    ('rawElevationVelocity', RKFourByte),
    ('rawAzimuthVelocity', RKFourByte),
    ('rawElevationStatus', RKFourByte),
    ('rawAzimuthStatus', RKFourByte),
    ('queueSize', uint8_t),
    ('elevationMode', uint8_t),
    ('azimuthMode', uint8_t),
    ('sequence', uint8_t),
    ('flag', RKPositionFlag),
    ('elevationDegrees', c_float),
    ('azimuthDegrees', c_float),
    ('elevationVelocityDegreesPerSecond', c_float),
    ('azimuthVelocityDegreesPerSecond', c_float),
    ('reserved1', c_float),
    ('reserved2', c_float),
    ('reserved3', c_float),
    ('reserved4', c_float),
    ('sweepElevationDegrees', c_float),
    ('sweepAzimuthDegrees', c_float),
    ('time', struct_timeval),
    ('timeDouble', c_double),
    ('volumeIndex', uint32_t),
    ('sweepIndex', uint32_t),
]

# RKTypes.h: 1395
class union_rk_position(Union):
    pass

union_rk_position._pack_ = 1
union_rk_position.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_position._anonymous_ = [
    'unnamed_1',
]
union_rk_position._fields_ = [
    ('unnamed_1', struct_anon_115),
    ('bytes', RKByte * int(128)),
]

RKPosition = union_rk_position# RKTypes.h: 1395

# RKTypes.h: 1402
class struct_rk_scan_action(Structure):
    pass

struct_rk_scan_action._pack_ = 1
struct_rk_scan_action.__slots__ = [
    'mode',
    'value',
    'sweepElevation',
    'sweepAzimuth',
]
struct_rk_scan_action._fields_ = [
    ('mode', RKPedestalInstructType * int(2)),
    ('value', c_float * int(2)),
    ('sweepElevation', c_float),
    ('sweepAzimuth', c_float),
]

RKScanAction = struct_rk_scan_action# RKTypes.h: 1402

# RKTypes.h: 1405
class struct_anon_116(Structure):
    pass

struct_anon_116._pack_ = 1
struct_anon_116.__slots__ = [
    'i',
    'n',
    't',
    's',
    'capacity',
    'gateCount',
    'downSampledGateCount',
    'pulseWidthSampleCount',
    'marker',
    'time',
    'timeDouble',
    'rawAzimuth',
    'rawElevation',
    'configIndex',
    'configSubIndex',
    'positionIndex',
    'gateSizeMeters',
    'elevationDegrees',
    'azimuthDegrees',
    'elevationVelocityDegreesPerSecond',
    'azimuthVelocityDegreesPerSecond',
    'compressorDataType',
]
struct_anon_116._fields_ = [
    ('i', RKIdentifier),
    ('n', RKIdentifier),
    ('t', uint64_t),
    ('s', RKPulseStatus),
    ('capacity', uint32_t),
    ('gateCount', uint32_t),
    ('downSampledGateCount', uint32_t),
    ('pulseWidthSampleCount', uint32_t),
    ('marker', RKMarker),
    ('time', struct_timeval),
    ('timeDouble', c_double),
    ('rawAzimuth', RKFourByte),
    ('rawElevation', RKFourByte),
    ('configIndex', uint16_t),
    ('configSubIndex', uint16_t),
    ('positionIndex', uint32_t),
    ('gateSizeMeters', c_float),
    ('elevationDegrees', c_float),
    ('azimuthDegrees', c_float),
    ('elevationVelocityDegreesPerSecond', c_float),
    ('azimuthVelocityDegreesPerSecond', c_float),
    ('compressorDataType', RKCompressorOption),
]

# RKTypes.h: 1430
class union_rk_pulse_header(Union):
    pass

union_rk_pulse_header._pack_ = 1
union_rk_pulse_header.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_pulse_header._anonymous_ = [
    'unnamed_1',
]
union_rk_pulse_header._fields_ = [
    ('unnamed_1', struct_anon_116),
    ('bytes', RKByte * int(192)),
]

RKPulseHeader = union_rk_pulse_header# RKTypes.h: 1430

# RKTypes.h: 1440
class struct_rk_pulse_parameters(Structure):
    pass

struct_rk_pulse_parameters._pack_ = 1
struct_rk_pulse_parameters.__slots__ = [
    'gid',
    'filterCounts',
    'planIndices',
    'planSizes',
]
struct_rk_pulse_parameters._fields_ = [
    ('gid', uint32_t),
    ('filterCounts', uint32_t * int(2)),
    ('planIndices', (uint32_t * int(8)) * int(2)),
    ('planSizes', (uint32_t * int(8)) * int(2)),
]

RKPulseParameters = struct_rk_pulse_parameters# RKTypes.h: 1440

# RKTypes.h: 1449
class struct_anon_117(Structure):
    pass

struct_anon_117._pack_ = 1
struct_anon_117.__slots__ = [
    'header',
    'parameters',
]
struct_anon_117._fields_ = [
    ('header', RKPulseHeader),
    ('parameters', RKPulseParameters),
]

# RKTypes.h: 1448
class union_anon_118(Union):
    pass

union_anon_118._pack_ = 1
union_anon_118.__slots__ = [
    'unnamed_1',
    'headerBytes',
]
union_anon_118._anonymous_ = [
    'unnamed_1',
]
union_anon_118._fields_ = [
    ('unnamed_1', struct_anon_117),
    ('headerBytes', RKByte * int(384)),
]

# RKTypes.h: 1456
class struct_rk_pulse(Structure):
    pass

struct_rk_pulse._pack_ = 1
struct_rk_pulse.__slots__ = [
    'unnamed_1',
    'data',
]
struct_rk_pulse._anonymous_ = [
    'unnamed_1',
]
struct_rk_pulse._fields_ = [
    ('unnamed_1', union_anon_118),
    ('data', RKByte * int(0)),
]

RKPulse = struct_rk_pulse# RKTypes.h: 1456

# RKTypes.h: 1488
class struct_rk_ray_header(Structure):
    pass

struct_rk_ray_header._pack_ = 1
struct_rk_ray_header.__slots__ = [
    'capacity',
    's',
    'i',
    'n',
    'marker',
    'momentList',
    'productList',
    'configIndex',
    'configSubIndex',
    'gateCount',
    'pulseCount',
    'gateSizeMeters',
    'sweepElevation',
    'sweepAzimuth',
    'startTime',
    'startTimeDouble',
    'startAzimuth',
    'startElevation',
    'endTime',
    'endTimeDouble',
    'endAzimuth',
    'endElevation',
    'fftOrder',
    'reserved1',
    'reserved2',
    'reserved3',
]
struct_rk_ray_header._fields_ = [
    ('capacity', uint32_t),
    ('s', RKRayStatus),
    ('i', RKIdentifier),
    ('n', RKIdentifier),
    ('marker', RKMarker),
    ('momentList', RKMomentList),
    ('productList', RKProductList),
    ('configIndex', uint16_t),
    ('configSubIndex', uint16_t),
    ('gateCount', uint16_t),
    ('pulseCount', uint16_t),
    ('gateSizeMeters', c_float),
    ('sweepElevation', c_float),
    ('sweepAzimuth', c_float),
    ('startTime', struct_timeval),
    ('startTimeDouble', c_double),
    ('startAzimuth', c_float),
    ('startElevation', c_float),
    ('endTime', struct_timeval),
    ('endTimeDouble', c_double),
    ('endAzimuth', c_float),
    ('endElevation', c_float),
    ('fftOrder', uint8_t),
    ('reserved1', uint8_t),
    ('reserved2', uint8_t),
    ('reserved3', uint8_t),
]

RKRayHeader = struct_rk_ray_header# RKTypes.h: 1488

# RKTypes.h: 1496
class union_anon_119(Union):
    pass

union_anon_119._pack_ = 1
union_anon_119.__slots__ = [
    'header',
    'headerBytes',
]
union_anon_119._fields_ = [
    ('header', RKRayHeader),
    ('headerBytes', RKByte * int(128)),
]

# RKTypes.h: 1501
class struct_rk_ray(Structure):
    pass

struct_rk_ray._pack_ = 1
struct_rk_ray.__slots__ = [
    'unnamed_1',
    'data',
]
struct_rk_ray._anonymous_ = [
    'unnamed_1',
]
struct_rk_ray._fields_ = [
    ('unnamed_1', union_anon_119),
    ('data', RKByte * int(0)),
]

RKRay = struct_rk_ray# RKTypes.h: 1501

# RKTypes.h: 1521
class struct_rk_sweep_header(Structure):
    pass

struct_rk_sweep_header._pack_ = 1
struct_rk_sweep_header.__slots__ = [
    'i',
    'rayCount',
    'gateCount',
    'startTime',
    'endTime',
    'momentList',
    'productList',
    'gateSizeMeters',
    'isPPI',
    'isRHI',
    'external',
    'desc',
    'config',
    'filename',
]
struct_rk_sweep_header._fields_ = [
    ('i', RKIdentifier),
    ('rayCount', uint32_t),
    ('gateCount', uint32_t),
    ('startTime', c_double),
    ('endTime', c_double),
    ('momentList', RKMomentList),
    ('productList', RKProductList),
    ('gateSizeMeters', c_float),
    ('isPPI', c_bool),
    ('isRHI', c_bool),
    ('external', c_bool),
    ('desc', RKRadarDesc),
    ('config', RKConfig),
    ('filename', c_char * int((1024 - 80))),
]

RKSweepHeader = struct_rk_sweep_header# RKTypes.h: 1521

# RKTypes.h: 1530
class struct_rk_sweep(Structure):
    pass

struct_rk_sweep._pack_ = 1
struct_rk_sweep.__slots__ = [
    'header',
    'rayBuffer',
    'rays',
]
struct_rk_sweep._fields_ = [
    ('header', RKSweepHeader),
    ('rayBuffer', RKBuffer),
    ('rays', POINTER(RKRay) * int(1500)),
]

RKSweep = struct_rk_sweep# RKTypes.h: 1530

# RKTypes.h: 1533
class struct_anon_120(Structure):
    pass

struct_anon_120._pack_ = 1
struct_anon_120.__slots__ = [
    'preface',
    'format',
    'dataType',
    'reserved',
    'desc',
    'config',
]
struct_anon_120._fields_ = [
    ('preface', RKName),
    ('format', uint32_t),
    ('dataType', RKRawDataType),
    ('reserved', uint8_t * int(123)),
    ('desc', RKRadarDesc),
    ('config', RKConfig),
]

# RKTypes.h: 1542
class union_rk_file_header(Union):
    pass

union_rk_file_header._pack_ = 1
union_rk_file_header.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_file_header._anonymous_ = [
    'unnamed_1',
]
union_rk_file_header._fields_ = [
    ('unnamed_1', struct_anon_120),
    ('bytes', RKByte * int(4096)),
]

RKFileHeader = union_rk_file_header# RKTypes.h: 1542

# RKTypes.h: 1556
class struct_rk_preferene_object(Structure):
    pass

struct_rk_preferene_object._pack_ = 1
struct_rk_preferene_object.__slots__ = [
    'keyword',
    'valueString',
    'isNumeric',
    'isValid',
    'numericCount',
    'subStrings',
    'doubleValues',
    'boolValues',
]
struct_rk_preferene_object._fields_ = [
    ('keyword', c_char * int(128)),
    ('valueString', c_char * int(4096)),
    ('isNumeric', c_bool),
    ('isValid', c_bool),
    ('numericCount', c_int),
    ('subStrings', (c_char * int(128)) * int(4)),
    ('doubleValues', c_double * int(4)),
    ('boolValues', c_bool * int(4)),
]

RKPreferenceObject = struct_rk_preferene_object# RKTypes.h: 1556

# RKTypes.h: 1567
class struct_rk_control(Structure):
    pass

struct_rk_control._pack_ = 1
struct_rk_control.__slots__ = [
    'uid',
    'state',
    'level',
    'label',
    'command',
]
struct_rk_control._fields_ = [
    ('uid', uint32_t),
    ('state', uint8_t),
    ('level', uint8_t),
    ('label', c_char * int(128)),
    ('command', c_char * int(512)),
]

RKControl = struct_rk_control# RKTypes.h: 1567

# RKTypes.h: 1591
class struct_rk_status(Structure):
    pass

struct_rk_status._pack_ = 1
struct_rk_status.__slots__ = [
    'i',
    'flag',
    'memoryUsage',
    'pulseMonitorLag',
    'pulseSkipCount',
    'pulseCoreLags',
    'pulseCoreUsage',
    'ringMonitorLag',
    'ringSkipCount',
    'ringCoreLags',
    'ringCoreUsage',
    'rayMonitorLag',
    'raySkipCount',
    'rayCoreLags',
    'rayCoreUsage',
    'recorderLag',
]
struct_rk_status._fields_ = [
    ('i', RKIdentifier),
    ('flag', RKStatusFlag),
    ('memoryUsage', c_size_t),
    ('pulseMonitorLag', uint8_t),
    ('pulseSkipCount', uint8_t),
    ('pulseCoreLags', uint8_t * int(16)),
    ('pulseCoreUsage', uint8_t * int(16)),
    ('ringMonitorLag', uint8_t),
    ('ringSkipCount', uint8_t),
    ('ringCoreLags', uint8_t * int(16)),
    ('ringCoreUsage', uint8_t * int(16)),
    ('rayMonitorLag', uint8_t),
    ('raySkipCount', uint8_t),
    ('rayCoreLags', uint8_t * int(16)),
    ('rayCoreUsage', uint8_t * int(16)),
    ('recorderLag', uint8_t),
]

RKStatus = struct_rk_status# RKTypes.h: 1591

# RKTypes.h: 1605
class struct_rk_simple_engine(Structure):
    pass

struct_rk_simple_engine._pack_ = 1
struct_rk_simple_engine.__slots__ = [
    'name',
    'verbose',
    'tid',
    'state',
    'memoryUsage',
    'userResource',
]
struct_rk_simple_engine._fields_ = [
    ('name', RKName),
    ('verbose', uint8_t),
    ('tid', pthread_t),
    ('state', RKEngineState),
    ('memoryUsage', uint32_t),
    ('userResource', POINTER(None)),
]

RKSimpleEngine = struct_rk_simple_engine# RKTypes.h: 1605

# RKTypes.h: 1619
class struct_rk_file_monitor(Structure):
    pass

struct_rk_file_monitor._pack_ = 1
struct_rk_file_monitor.__slots__ = [
    'name',
    'verbose',
    'tid',
    'state',
    'memoryUsage',
    'filename',
    'callbackRoutine',
    'userResource',
]
struct_rk_file_monitor._fields_ = [
    ('name', RKName),
    ('verbose', uint8_t),
    ('tid', pthread_t),
    ('state', RKEngineState),
    ('memoryUsage', uint32_t),
    ('filename', c_char * int(1024)),
    ('callbackRoutine', CFUNCTYPE(UNCHECKED(None), POINTER(None))),
    ('userResource', POINTER(None)),
]

RKFileMonitor = struct_rk_file_monitor# RKTypes.h: 1619

# RKTypes.h: 1626
class struct_anon_121(Structure):
    pass

struct_anon_121._pack_ = 1
struct_anon_121.__slots__ = [
    'key',
    'name',
    'unit',
    'description',
    'colormap',
    'symbol',
    'index',
    'type',
    'pieceCount',
    'w',
    'b',
    'l',
    'mininimumValue',
    'maximumValue',
    'cfScale',
    'cfOffset',
]
struct_anon_121._fields_ = [
    ('key', uint32_t),
    ('name', RKName),
    ('unit', RKName),
    ('description', RKName),
    ('colormap', RKName),
    ('symbol', c_char * int(8)),
    ('index', RKProductIndex),
    ('type', RKProductType),
    ('pieceCount', uint32_t),
    ('w', RKFloat * int(16)),
    ('b', RKFloat * int(16)),
    ('l', RKFloat * int(16)),
    ('mininimumValue', RKFloat),
    ('maximumValue', RKFloat),
    ('cfScale', c_float),
    ('cfOffset', c_float),
]

# RKTypes.h: 1645
class union_rk_product_desc(Union):
    pass

union_rk_product_desc._pack_ = 1
union_rk_product_desc.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_product_desc._anonymous_ = [
    'unnamed_1',
]
union_rk_product_desc._fields_ = [
    ('unnamed_1', struct_anon_121),
    ('bytes', RKByte * int(1024)),
]

RKProductDesc = union_rk_product_desc# RKTypes.h: 1645

# RKTypes.h: 1648
class struct_anon_122(Structure):
    pass

struct_anon_122._pack_ = 1
struct_anon_122.__slots__ = [
    'radarName',
    'latitude',
    'longitude',
    'altitude',
    'heading',
    'radarHeight',
    'wavelength',
    'sweepElevation',
    'sweepAzimuth',
    'volumeIndex',
    'sweepIndex',
    'rayCount',
    'gateCount',
    'gateSizeMeters',
    'startTime',
    'endTime',
    'isPPI',
    'isRHI',
    'prt',
    'pw',
    'noise',
    'systemZCal',
    'systemDCal',
    'systemPCal',
    'ZCal',
    'DCal',
    'PCal',
    'SNRThreshold',
    'SQIThreshold',
    'waveformName',
    'momentMethod',
    'vcpDefinition',
    'suggestedFilename',
]
struct_anon_122._fields_ = [
    ('radarName', RKName),
    ('latitude', c_double),
    ('longitude', c_double),
    ('altitude', c_double),
    ('heading', c_float),
    ('radarHeight', c_float),
    ('wavelength', c_float),
    ('sweepElevation', c_float),
    ('sweepAzimuth', c_float),
    ('volumeIndex', uint32_t),
    ('sweepIndex', uint32_t),
    ('rayCount', uint32_t),
    ('gateCount', uint32_t),
    ('gateSizeMeters', c_float),
    ('startTime', c_double),
    ('endTime', c_double),
    ('isPPI', c_bool),
    ('isRHI', c_bool),
    ('prt', RKFloat * int(8)),
    ('pw', RKFloat * int(8)),
    ('noise', RKFloat * int(2)),
    ('systemZCal', RKFloat * int(2)),
    ('systemDCal', RKFloat),
    ('systemPCal', RKFloat),
    ('ZCal', (RKFloat * int(2)) * int(8)),
    ('DCal', RKFloat * int(8)),
    ('PCal', RKFloat * int(8)),
    ('SNRThreshold', RKFloat),
    ('SQIThreshold', RKFloat),
    ('waveformName', RKName),
    ('momentMethod', RKName),
    ('vcpDefinition', c_char * int(512)),
    ('suggestedFilename', c_char * int(1024)),
]

# RKTypes.h: 1684
class union_rk_product_header(Union):
    pass

union_rk_product_header._pack_ = 1
union_rk_product_header.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_product_header._anonymous_ = [
    'unnamed_1',
]
union_rk_product_header._fields_ = [
    ('unnamed_1', struct_anon_122),
    ('bytes', RKByte * int(2048)),
]

RKProductHeader = union_rk_product_header# RKTypes.h: 1684

# RKTypes.h: 1701
class struct_rk_product(Structure):
    pass

struct_rk_product._pack_ = 1
struct_rk_product.__slots__ = [
    'i',
    'pid',
    'desc',
    'flag',
    'header',
    'capacity',
    'totalBufferSize',
    'startAzimuth',
    'endAzimuth',
    'startElevation',
    'endElevation',
    'startTime',
    'endTime',
    'data',
]
struct_rk_product._fields_ = [
    ('i', RKIdentifier),
    ('pid', RKProductId),
    ('desc', RKProductDesc),
    ('flag', RKProductStatus),
    ('header', RKProductHeader),
    ('capacity', uint32_t),
    ('totalBufferSize', uint32_t),
    ('startAzimuth', POINTER(RKFloat)),
    ('endAzimuth', POINTER(RKFloat)),
    ('startElevation', POINTER(RKFloat)),
    ('endElevation', POINTER(RKFloat)),
    ('startTime', POINTER(c_double)),
    ('endTime', POINTER(c_double)),
    ('data', POINTER(RKFloat)),
]

RKProduct = struct_rk_product# RKTypes.h: 1701

# RKTypes.h: 1706
class struct_rk_product_collection(Structure):
    pass

struct_rk_product_collection._pack_ = 1
struct_rk_product_collection.__slots__ = [
    'count',
    'products',
]
struct_rk_product_collection._fields_ = [
    ('count', uint32_t),
    ('products', POINTER(RKProduct)),
]

RKProductCollection = struct_rk_product_collection# RKTypes.h: 1706

# RKTypes.h: 1715
class struct_rk_iir_filter(Structure):
    pass

struct_rk_iir_filter._pack_ = 1
struct_rk_iir_filter.__slots__ = [
    'name',
    'type',
    'bLength',
    'aLength',
    'B',
    'A',
]
struct_rk_iir_filter._fields_ = [
    ('name', RKName),
    ('type', RKFilterType),
    ('bLength', uint32_t),
    ('aLength', uint32_t),
    ('B', RKComplex * int(8)),
    ('A', RKComplex * int(8)),
]

RKIIRFilter = struct_rk_iir_filter# RKTypes.h: 1715

# RKTypes.h: 1721
class struct_rk_task(Structure):
    pass

struct_rk_task._pack_ = 1
struct_rk_task.__slots__ = [
    'command',
    'timeout',
    'endingEvent',
]
struct_rk_task._fields_ = [
    ('command', RKCommand),
    ('timeout', c_double),
    ('endingEvent', RKEventType),
]

RKTask = struct_rk_task# RKTypes.h: 1721

# RKTypes.h: 1733
class struct_rk_command_queue(Structure):
    pass

struct_rk_command_queue._pack_ = 1
struct_rk_command_queue.__slots__ = [
    'head',
    'tail',
    'count',
    'depth',
    'nonblocking',
    'buffer',
    'lock',
    'tic',
    'toc',
]
struct_rk_command_queue._fields_ = [
    ('head', uint16_t),
    ('tail', uint16_t),
    ('count', uint16_t),
    ('depth', uint16_t),
    ('nonblocking', c_bool),
    ('buffer', POINTER(RKCommand)),
    ('lock', pthread_mutex_t),
    ('tic', uint32_t),
    ('toc', uint32_t),
]

RKCommandQueue = struct_rk_command_queue# RKTypes.h: 1733

sa_family_t = c_ushort# /usr/include/x86_64-linux-gnu/bits/sockaddr.h: 28

# /usr/include/x86_64-linux-gnu/bits/socket.h: 183
class struct_sockaddr(Structure):
    pass

struct_sockaddr.__slots__ = [
    'sa_family',
    'sa_data',
]
struct_sockaddr._fields_ = [
    ('sa_family', sa_family_t),
    ('sa_data', c_char * int(14)),
]

in_addr_t = uint32_t# /usr/include/netinet/in.h: 30

# /usr/include/netinet/in.h: 31
class struct_in_addr(Structure):
    pass

struct_in_addr.__slots__ = [
    's_addr',
]
struct_in_addr._fields_ = [
    ('s_addr', in_addr_t),
]

in_port_t = uint16_t# /usr/include/netinet/in.h: 123

# /usr/include/netinet/in.h: 245
class struct_sockaddr_in(Structure):
    pass

struct_sockaddr_in.__slots__ = [
    'sin_family',
    'sin_port',
    'sin_addr',
    'sin_zero',
]
struct_sockaddr_in._fields_ = [
    ('sin_family', sa_family_t),
    ('sin_port', in_port_t),
    ('sin_addr', struct_in_addr),
    ('sin_zero', c_ubyte * int((((sizeof(struct_sockaddr) - sizeof(c_ushort)) - sizeof(in_port_t)) - sizeof(struct_in_addr)))),
]

enum_RKJSONObjectType = c_int# RKMisc.h: 113

RKJSONObjectTypeUnknown = 0# RKMisc.h: 113

RKJSONObjectTypePlain = (RKJSONObjectTypeUnknown + 1)# RKMisc.h: 113

RKJSONObjectTypeString = (RKJSONObjectTypePlain + 1)# RKMisc.h: 113

RKJSONObjectTypeArray = (RKJSONObjectTypeString + 1)# RKMisc.h: 113

RKJSONObjectTypeObject = (RKJSONObjectTypeArray + 1)# RKMisc.h: 113

# RKMisc.h: 126
if _libs["radarkit"].has("RKGetColor", "cdecl"):
    RKGetColor = _libs["radarkit"].get("RKGetColor", "cdecl")
    RKGetColor.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetColor.restype = ReturnString
    else:
        RKGetColor.restype = String
        RKGetColor.errcheck = ReturnString

# RKMisc.h: 129
if _libs["radarkit"].has("RKGetColorOfIndex", "cdecl"):
    RKGetColorOfIndex = _libs["radarkit"].get("RKGetColorOfIndex", "cdecl")
    RKGetColorOfIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetColorOfIndex.restype = ReturnString
    else:
        RKGetColorOfIndex.restype = String
        RKGetColorOfIndex.errcheck = ReturnString

# RKMisc.h: 132
if _libs["radarkit"].has("RKGetBackgroundColor", "cdecl"):
    RKGetBackgroundColor = _libs["radarkit"].get("RKGetBackgroundColor", "cdecl")
    RKGetBackgroundColor.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColor.restype = ReturnString
    else:
        RKGetBackgroundColor.restype = String
        RKGetBackgroundColor.errcheck = ReturnString

# RKMisc.h: 135
if _libs["radarkit"].has("RKGetBackgroundColorOfIndex", "cdecl"):
    RKGetBackgroundColorOfIndex = _libs["radarkit"].get("RKGetBackgroundColorOfIndex", "cdecl")
    RKGetBackgroundColorOfIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColorOfIndex.restype = ReturnString
    else:
        RKGetBackgroundColorOfIndex.restype = String
        RKGetBackgroundColorOfIndex.errcheck = ReturnString

# RKMisc.h: 138
if _libs["radarkit"].has("RKGetBackgroundColorOfCubeIndex", "cdecl"):
    RKGetBackgroundColorOfCubeIndex = _libs["radarkit"].get("RKGetBackgroundColorOfCubeIndex", "cdecl")
    RKGetBackgroundColorOfCubeIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColorOfCubeIndex.restype = ReturnString
    else:
        RKGetBackgroundColorOfCubeIndex.restype = String
        RKGetBackgroundColorOfCubeIndex.errcheck = ReturnString

# RKMisc.h: 148
if _libs["radarkit"].has("RKExtractJSON", "cdecl"):
    RKExtractJSON = _libs["radarkit"].get("RKExtractJSON", "cdecl")
    RKExtractJSON.argtypes = [String, POINTER(uint8_t), String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKExtractJSON.restype = ReturnString
    else:
        RKExtractJSON.restype = String
        RKExtractJSON.errcheck = ReturnString

# RKMisc.h: 151
if _libs["radarkit"].has("RKGetValueOfKey", "cdecl"):
    RKGetValueOfKey = _libs["radarkit"].get("RKGetValueOfKey", "cdecl")
    RKGetValueOfKey.argtypes = [String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetValueOfKey.restype = ReturnString
    else:
        RKGetValueOfKey.restype = String
        RKGetValueOfKey.errcheck = ReturnString

# RKMisc.h: 154
if _libs["radarkit"].has("RKReplaceAllValuesOfKey", "cdecl"):
    RKReplaceAllValuesOfKey = _libs["radarkit"].get("RKReplaceAllValuesOfKey", "cdecl")
    RKReplaceAllValuesOfKey.argtypes = [String, String, c_int]
    RKReplaceAllValuesOfKey.restype = None

# RKMisc.h: 157
if _libs["radarkit"].has("RKReplaceEnumOfKey", "cdecl"):
    RKReplaceEnumOfKey = _libs["radarkit"].get("RKReplaceEnumOfKey", "cdecl")
    RKReplaceEnumOfKey.argtypes = [String, String, c_int]
    RKReplaceEnumOfKey.restype = None

# RKMisc.h: 160
if _libs["radarkit"].has("RKReviseLogicalValues", "cdecl"):
    RKReviseLogicalValues = _libs["radarkit"].get("RKReviseLogicalValues", "cdecl")
    RKReviseLogicalValues.argtypes = [String]
    RKReviseLogicalValues.restype = None

# RKMisc.h: 172
if _libs["radarkit"].has("RKJSONSkipWhiteSpaces", "cdecl"):
    RKJSONSkipWhiteSpaces = _libs["radarkit"].get("RKJSONSkipWhiteSpaces", "cdecl")
    RKJSONSkipWhiteSpaces.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONSkipWhiteSpaces.restype = ReturnString
    else:
        RKJSONSkipWhiteSpaces.restype = String
        RKJSONSkipWhiteSpaces.errcheck = ReturnString

# RKMisc.h: 178
if _libs["radarkit"].has("RKJSONScanPassed", "cdecl"):
    RKJSONScanPassed = _libs["radarkit"].get("RKJSONScanPassed", "cdecl")
    RKJSONScanPassed.argtypes = [String, String, c_char]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONScanPassed.restype = ReturnString
    else:
        RKJSONScanPassed.restype = String
        RKJSONScanPassed.errcheck = ReturnString

# RKMisc.h: 181
if _libs["radarkit"].has("RKJSONGetElement", "cdecl"):
    RKJSONGetElement = _libs["radarkit"].get("RKJSONGetElement", "cdecl")
    RKJSONGetElement.argtypes = [String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONGetElement.restype = ReturnString
    else:
        RKJSONGetElement.restype = String
        RKJSONGetElement.errcheck = ReturnString

# RKMisc.h: 185
if _libs["radarkit"].has("RKJSONKeyValueFromElement", "cdecl"):
    RKJSONKeyValueFromElement = _libs["radarkit"].get("RKJSONKeyValueFromElement", "cdecl")
    RKJSONKeyValueFromElement.argtypes = [String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONKeyValueFromElement.restype = ReturnString
    else:
        RKJSONKeyValueFromElement.restype = String
        RKJSONKeyValueFromElement.errcheck = ReturnString

# RKMisc.h: 194
if _libs["radarkit"].has("RKJSONGetValueOfKey", "cdecl"):
    RKJSONGetValueOfKey = _libs["radarkit"].get("RKJSONGetValueOfKey", "cdecl")
    RKJSONGetValueOfKey.argtypes = [String, String, String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONGetValueOfKey.restype = ReturnString
    else:
        RKJSONGetValueOfKey.restype = String
        RKJSONGetValueOfKey.errcheck = ReturnString

# RKMisc.h: 201
if _libs["radarkit"].has("RKUIntegerToCommaStyleString", "cdecl"):
    RKUIntegerToCommaStyleString = _libs["radarkit"].get("RKUIntegerToCommaStyleString", "cdecl")
    RKUIntegerToCommaStyleString.argtypes = [c_ulonglong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKUIntegerToCommaStyleString.restype = ReturnString
    else:
        RKUIntegerToCommaStyleString.restype = String
        RKUIntegerToCommaStyleString.errcheck = ReturnString

# RKMisc.h: 204
if _libs["radarkit"].has("RKIntegerToCommaStyleString", "cdecl"):
    RKIntegerToCommaStyleString = _libs["radarkit"].get("RKIntegerToCommaStyleString", "cdecl")
    RKIntegerToCommaStyleString.argtypes = [c_longlong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKIntegerToCommaStyleString.restype = ReturnString
    else:
        RKIntegerToCommaStyleString.restype = String
        RKIntegerToCommaStyleString.errcheck = ReturnString

# RKMisc.h: 207
if _libs["radarkit"].has("RKIntegerToHexStyleString", "cdecl"):
    RKIntegerToHexStyleString = _libs["radarkit"].get("RKIntegerToHexStyleString", "cdecl")
    RKIntegerToHexStyleString.argtypes = [c_longlong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKIntegerToHexStyleString.restype = ReturnString
    else:
        RKIntegerToHexStyleString.restype = String
        RKIntegerToHexStyleString.errcheck = ReturnString

# RKMisc.h: 210
if _libs["radarkit"].has("RKFloatToCommaStyleStringAndDecimals", "cdecl"):
    RKFloatToCommaStyleStringAndDecimals = _libs["radarkit"].get("RKFloatToCommaStyleStringAndDecimals", "cdecl")
    RKFloatToCommaStyleStringAndDecimals.argtypes = [c_double, c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFloatToCommaStyleStringAndDecimals.restype = ReturnString
    else:
        RKFloatToCommaStyleStringAndDecimals.restype = String
        RKFloatToCommaStyleStringAndDecimals.errcheck = ReturnString

# RKMisc.h: 213
if _libs["radarkit"].has("RKFloatToCommaStyleString", "cdecl"):
    RKFloatToCommaStyleString = _libs["radarkit"].get("RKFloatToCommaStyleString", "cdecl")
    RKFloatToCommaStyleString.argtypes = [c_double]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFloatToCommaStyleString.restype = ReturnString
    else:
        RKFloatToCommaStyleString.restype = String
        RKFloatToCommaStyleString.errcheck = ReturnString

# RKMisc.h: 220
if _libs["radarkit"].has("RKNow", "cdecl"):
    RKNow = _libs["radarkit"].get("RKNow", "cdecl")
    RKNow.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKNow.restype = ReturnString
    else:
        RKNow.restype = String
        RKNow.errcheck = ReturnString

# RKMisc.h: 223
if _libs["radarkit"].has("RKTimeStringInISOFormatToTimeval", "cdecl"):
    RKTimeStringInISOFormatToTimeval = _libs["radarkit"].get("RKTimeStringInISOFormatToTimeval", "cdecl")
    RKTimeStringInISOFormatToTimeval.argtypes = [String]
    RKTimeStringInISOFormatToTimeval.restype = struct_timeval

# RKMisc.h: 226
if _libs["radarkit"].has("RKTimeStringISOToTimeDouble", "cdecl"):
    RKTimeStringISOToTimeDouble = _libs["radarkit"].get("RKTimeStringISOToTimeDouble", "cdecl")
    RKTimeStringISOToTimeDouble.argtypes = [String]
    RKTimeStringISOToTimeDouble.restype = c_double

# RKMisc.h: 229
if _libs["radarkit"].has("RKTimevalDiff", "cdecl"):
    RKTimevalDiff = _libs["radarkit"].get("RKTimevalDiff", "cdecl")
    RKTimevalDiff.argtypes = [struct_timeval, struct_timeval]
    RKTimevalDiff.restype = c_double

# RKMisc.h: 232
if _libs["radarkit"].has("RKTimespecDiff", "cdecl"):
    RKTimespecDiff = _libs["radarkit"].get("RKTimespecDiff", "cdecl")
    RKTimespecDiff.argtypes = [struct_timespec, struct_timespec]
    RKTimespecDiff.restype = c_double

# RKMisc.h: 235
if _libs["radarkit"].has("RKUTCTime", "cdecl"):
    RKUTCTime = _libs["radarkit"].get("RKUTCTime", "cdecl")
    RKUTCTime.argtypes = [POINTER(struct_timespec)]
    RKUTCTime.restype = None

# RKMisc.h: 239
if _libs["radarkit"].has("RKTimevalToString", "cdecl"):
    RKTimevalToString = _libs["radarkit"].get("RKTimevalToString", "cdecl")
    RKTimevalToString.argtypes = [struct_timeval, c_int, c_bool]
    if sizeof(c_int) == sizeof(c_void_p):
        RKTimevalToString.restype = ReturnString
    else:
        RKTimevalToString.restype = String
        RKTimevalToString.errcheck = ReturnString

# RKMisc.h: 242
if _libs["radarkit"].has("RKTimeDoubleToString", "cdecl"):
    RKTimeDoubleToString = _libs["radarkit"].get("RKTimeDoubleToString", "cdecl")
    RKTimeDoubleToString.argtypes = [c_double, c_int, c_bool]
    if sizeof(c_int) == sizeof(c_void_p):
        RKTimeDoubleToString.restype = ReturnString
    else:
        RKTimeDoubleToString.restype = String
        RKTimeDoubleToString.errcheck = ReturnString

# RKMisc.h: 248
if _libs["radarkit"].has("RKFilenameExists", "cdecl"):
    RKFilenameExists = _libs["radarkit"].get("RKFilenameExists", "cdecl")
    RKFilenameExists.argtypes = [String]
    RKFilenameExists.restype = c_bool

# RKMisc.h: 249
if _libs["radarkit"].has("RKPreparePath", "cdecl"):
    RKPreparePath = _libs["radarkit"].get("RKPreparePath", "cdecl")
    RKPreparePath.argtypes = [String]
    RKPreparePath.restype = None

# RKMisc.h: 250
if _libs["radarkit"].has("RKCountFilesInPath", "cdecl"):
    RKCountFilesInPath = _libs["radarkit"].get("RKCountFilesInPath", "cdecl")
    RKCountFilesInPath.argtypes = [String]
    RKCountFilesInPath.restype = c_long

# RKMisc.h: 251
if _libs["radarkit"].has("RKFolderOfFilename", "cdecl"):
    RKFolderOfFilename = _libs["radarkit"].get("RKFolderOfFilename", "cdecl")
    RKFolderOfFilename.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFolderOfFilename.restype = ReturnString
    else:
        RKFolderOfFilename.restype = String
        RKFolderOfFilename.errcheck = ReturnString

# RKMisc.h: 252
if _libs["radarkit"].has("RKFileExtension", "cdecl"):
    RKFileExtension = _libs["radarkit"].get("RKFileExtension", "cdecl")
    RKFileExtension.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFileExtension.restype = ReturnString
    else:
        RKFileExtension.restype = String
        RKFileExtension.errcheck = ReturnString

# RKMisc.h: 253
if _libs["radarkit"].has("RKLastPartOfPath", "cdecl"):
    RKLastPartOfPath = _libs["radarkit"].get("RKLastPartOfPath", "cdecl")
    RKLastPartOfPath.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastPartOfPath.restype = ReturnString
    else:
        RKLastPartOfPath.restype = String
        RKLastPartOfPath.errcheck = ReturnString

# RKMisc.h: 254
if _libs["radarkit"].has("RKLastTwoPartsOfPath", "cdecl"):
    RKLastTwoPartsOfPath = _libs["radarkit"].get("RKLastTwoPartsOfPath", "cdecl")
    RKLastTwoPartsOfPath.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastTwoPartsOfPath.restype = ReturnString
    else:
        RKLastTwoPartsOfPath.restype = String
        RKLastTwoPartsOfPath.errcheck = ReturnString

# RKMisc.h: 255
if _libs["radarkit"].has("RKLastNPartsOfPath", "cdecl"):
    RKLastNPartsOfPath = _libs["radarkit"].get("RKLastNPartsOfPath", "cdecl")
    RKLastNPartsOfPath.argtypes = [String, c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastNPartsOfPath.restype = ReturnString
    else:
        RKLastNPartsOfPath.restype = String
        RKLastNPartsOfPath.errcheck = ReturnString

# RKMisc.h: 256
if _libs["radarkit"].has("RKPathStringByExpandingTilde", "cdecl"):
    RKPathStringByExpandingTilde = _libs["radarkit"].get("RKPathStringByExpandingTilde", "cdecl")
    RKPathStringByExpandingTilde.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPathStringByExpandingTilde.restype = ReturnString
    else:
        RKPathStringByExpandingTilde.restype = String
        RKPathStringByExpandingTilde.errcheck = ReturnString

# RKMisc.h: 257
if _libs["radarkit"].has("RKReplaceFileExtension", "cdecl"):
    RKReplaceFileExtension = _libs["radarkit"].get("RKReplaceFileExtension", "cdecl")
    RKReplaceFileExtension.argtypes = [String, String, String]
    RKReplaceFileExtension.restype = None

# RKMisc.h: 263
if _libs["radarkit"].has("RKSignalString", "cdecl"):
    RKSignalString = _libs["radarkit"].get("RKSignalString", "cdecl")
    RKSignalString.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSignalString.restype = ReturnString
    else:
        RKSignalString.restype = String
        RKSignalString.errcheck = ReturnString

# RKMisc.h: 269
if _libs["radarkit"].has("RKStripTail", "cdecl"):
    RKStripTail = _libs["radarkit"].get("RKStripTail", "cdecl")
    RKStripTail.argtypes = [String]
    RKStripTail.restype = c_int

# RKMisc.h: 270
if _libs["radarkit"].has("RKUnquote", "cdecl"):
    RKUnquote = _libs["radarkit"].get("RKUnquote", "cdecl")
    RKUnquote.argtypes = [String]
    RKUnquote.restype = c_int

# RKMisc.h: 271
if _libs["radarkit"].has("RKIndentCopy", "cdecl"):
    RKIndentCopy = _libs["radarkit"].get("RKIndentCopy", "cdecl")
    RKIndentCopy.argtypes = [String, String, c_int]
    RKIndentCopy.restype = c_int

# RKMisc.h: 272
if _libs["radarkit"].has("RKStringCenterized", "cdecl"):
    RKStringCenterized = _libs["radarkit"].get("RKStringCenterized", "cdecl")
    RKStringCenterized.argtypes = [String, String, c_int]
    RKStringCenterized.restype = c_int

# RKMisc.h: 273
if _libs["radarkit"].has("RKNextNoneWhite", "cdecl"):
    RKNextNoneWhite = _libs["radarkit"].get("RKNextNoneWhite", "cdecl")
    RKNextNoneWhite.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKNextNoneWhite.restype = ReturnString
    else:
        RKNextNoneWhite.restype = String
        RKNextNoneWhite.errcheck = ReturnString

# RKMisc.h: 274
if _libs["radarkit"].has("RKLastLine", "cdecl"):
    RKLastLine = _libs["radarkit"].get("RKLastLine", "cdecl")
    RKLastLine.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastLine.restype = ReturnString
    else:
        RKLastLine.restype = String
        RKLastLine.errcheck = ReturnString

# RKMisc.h: 275
if _libs["radarkit"].has("RKStripEscapeSequence", "cdecl"):
    RKStripEscapeSequence = _libs["radarkit"].get("RKStripEscapeSequence", "cdecl")
    RKStripEscapeSequence.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStripEscapeSequence.restype = ReturnString
    else:
        RKStripEscapeSequence.restype = String
        RKStripEscapeSequence.errcheck = ReturnString

# RKMisc.h: 281
if _libs["radarkit"].has("RKMinDiff", "cdecl"):
    RKMinDiff = _libs["radarkit"].get("RKMinDiff", "cdecl")
    RKMinDiff.argtypes = [c_float, c_float]
    RKMinDiff.restype = c_float

# RKMisc.h: 282
if _libs["radarkit"].has("RKUMinDiff", "cdecl"):
    RKUMinDiff = _libs["radarkit"].get("RKUMinDiff", "cdecl")
    RKUMinDiff.argtypes = [c_float, c_float]
    RKUMinDiff.restype = c_float

# RKMisc.h: 283
if _libs["radarkit"].has("RKModulo360Diff", "cdecl"):
    RKModulo360Diff = _libs["radarkit"].get("RKModulo360Diff", "cdecl")
    RKModulo360Diff.argtypes = [c_float, c_float]
    RKModulo360Diff.restype = c_float

# RKMisc.h: 284
if _libs["radarkit"].has("RKAngularCrossOver", "cdecl"):
    RKAngularCrossOver = _libs["radarkit"].get("RKAngularCrossOver", "cdecl")
    RKAngularCrossOver.argtypes = [c_float, c_float, c_float]
    RKAngularCrossOver.restype = c_bool

# RKMisc.h: 290
if _libs["radarkit"].has("RKGetCPUIndex", "cdecl"):
    RKGetCPUIndex = _libs["radarkit"].get("RKGetCPUIndex", "cdecl")
    RKGetCPUIndex.argtypes = [c_long]
    RKGetCPUIndex.restype = c_long

# RKMisc.h: 291
if _libs["radarkit"].has("RKGetMemoryUsage", "cdecl"):
    RKGetMemoryUsage = _libs["radarkit"].get("RKGetMemoryUsage", "cdecl")
    RKGetMemoryUsage.argtypes = []
    RKGetMemoryUsage.restype = c_long

# RKMisc.h: 297
if _libs["radarkit"].has("RKCountryFromPosition", "cdecl"):
    RKCountryFromPosition = _libs["radarkit"].get("RKCountryFromPosition", "cdecl")
    RKCountryFromPosition.argtypes = [c_double, c_double]
    if sizeof(c_int) == sizeof(c_void_p):
        RKCountryFromPosition.restype = ReturnString
    else:
        RKCountryFromPosition.restype = String
        RKCountryFromPosition.errcheck = ReturnString

# RKMisc.h: 303
if _libs["radarkit"].has("RKGetNextKeyValue", "cdecl"):
    RKGetNextKeyValue = _libs["radarkit"].get("RKGetNextKeyValue", "cdecl")
    RKGetNextKeyValue.argtypes = [String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetNextKeyValue.restype = ReturnString
    else:
        RKGetNextKeyValue.restype = String
        RKGetNextKeyValue.errcheck = ReturnString

# RKMisc.h: 304
if _libs["radarkit"].has("RKMergeColumns", "cdecl"):
    RKMergeColumns = _libs["radarkit"].get("RKMergeColumns", "cdecl")
    RKMergeColumns.argtypes = [String, String, String, c_int]
    RKMergeColumns.restype = c_int

# RKMisc.h: 310
if _libs["radarkit"].has("RKStringLower", "cdecl"):
    RKStringLower = _libs["radarkit"].get("RKStringLower", "cdecl")
    RKStringLower.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringLower.restype = ReturnString
    else:
        RKStringLower.restype = String
        RKStringLower.errcheck = ReturnString

# RKMisc.h: 311
if _libs["radarkit"].has("RKBytesInHex", "cdecl"):
    RKBytesInHex = _libs["radarkit"].get("RKBytesInHex", "cdecl")
    RKBytesInHex.argtypes = [String, POINTER(None), c_size_t]
    if sizeof(c_int) == sizeof(c_void_p):
        RKBytesInHex.restype = ReturnString
    else:
        RKBytesInHex.restype = String
        RKBytesInHex.errcheck = ReturnString

# RKMisc.h: 312
if _libs["radarkit"].has("RKBinaryString", "cdecl"):
    RKBinaryString = _libs["radarkit"].get("RKBinaryString", "cdecl")
    RKBinaryString.argtypes = [String, POINTER(None), c_size_t]
    if sizeof(c_int) == sizeof(c_void_p):
        RKBinaryString.restype = ReturnString
    else:
        RKBinaryString.restype = String
        RKBinaryString.errcheck = ReturnString

# RKMisc.h: 313
if _libs["radarkit"].has("RKHeadTailBytesInHex", "cdecl"):
    RKHeadTailBytesInHex = _libs["radarkit"].get("RKHeadTailBytesInHex", "cdecl")
    RKHeadTailBytesInHex.argtypes = [String, POINTER(None), c_size_t]
    RKHeadTailBytesInHex.restype = None

# RKMisc.h: 314
if _libs["radarkit"].has("RKHeadTailBinaryString", "cdecl"):
    RKHeadTailBinaryString = _libs["radarkit"].get("RKHeadTailBinaryString", "cdecl")
    RKHeadTailBinaryString.argtypes = [String, POINTER(None), c_size_t]
    RKHeadTailBinaryString.restype = None

# RKMisc.h: 315
if _libs["radarkit"].has("RKRadarHubPayloadString", "cdecl"):
    RKRadarHubPayloadString = _libs["radarkit"].get("RKRadarHubPayloadString", "cdecl")
    RKRadarHubPayloadString.argtypes = [String, POINTER(None), c_size_t]
    RKRadarHubPayloadString.restype = None

# RKMisc.h: 321
if _libs["radarkit"].has("RKSetArrayToFloat32", "cdecl"):
    RKSetArrayToFloat32 = _libs["radarkit"].get("RKSetArrayToFloat32", "cdecl")
    RKSetArrayToFloat32.argtypes = [POINTER(None), c_float, c_size_t]
    RKSetArrayToFloat32.restype = None

# RKFoundation.h: 93
class struct_RKGlobalParameterStruct(Structure):
    pass

struct_RKGlobalParameterStruct.__slots__ = [
    'program',
    'logfile',
    'logFolder',
    'rootDataFolder',
    'logTimeOnly',
    'dailyLog',
    'showColor',
    'statusColor',
    'convention',
    'lock',
    'stream',
]
struct_RKGlobalParameterStruct._fields_ = [
    ('program', c_char * int(32)),
    ('logfile', c_char * int(1024)),
    ('logFolder', c_char * int(256)),
    ('rootDataFolder', c_char * int(256)),
    ('logTimeOnly', c_bool),
    ('dailyLog', c_bool),
    ('showColor', c_bool),
    ('statusColor', c_bool),
    ('convention', RKSignalLocale),
    ('lock', pthread_mutex_t),
    ('stream', POINTER(FILE)),
]

RKGlobalParameters = struct_RKGlobalParameterStruct# RKFoundation.h: 93

# RKFoundation.h: 95
try:
    rkGlobalParameters = (RKGlobalParameters).in_dll(_libs["radarkit"], "rkGlobalParameters")
except:
    pass

# RKFoundation.h: 96
try:
    rkResultStrings = (POINTER(POINTER(c_char))).in_dll(_libs["radarkit"], "rkResultStrings")
except:
    pass

# RKFoundation.h: 101
if _libs["radarkit"].has("RKComplexAdd", "cdecl"):
    RKComplexAdd = _libs["radarkit"].get("RKComplexAdd", "cdecl")
    RKComplexAdd.argtypes = [RKComplex, RKComplex]
    RKComplexAdd.restype = RKComplex

# RKFoundation.h: 102
if _libs["radarkit"].has("RKComplexSubtract", "cdecl"):
    RKComplexSubtract = _libs["radarkit"].get("RKComplexSubtract", "cdecl")
    RKComplexSubtract.argtypes = [RKComplex, RKComplex]
    RKComplexSubtract.restype = RKComplex

# RKFoundation.h: 103
if _libs["radarkit"].has("RKComplexMultiply", "cdecl"):
    RKComplexMultiply = _libs["radarkit"].get("RKComplexMultiply", "cdecl")
    RKComplexMultiply.argtypes = [RKComplex, RKComplex]
    RKComplexMultiply.restype = RKComplex

# RKFoundation.h: 104
if _libs["radarkit"].has("RKComplexConjugate", "cdecl"):
    RKComplexConjugate = _libs["radarkit"].get("RKComplexConjugate", "cdecl")
    RKComplexConjugate.argtypes = [RKComplex]
    RKComplexConjugate.restype = RKComplex

# RKFoundation.h: 105
if _libs["radarkit"].has("RKComplexAbsSquare", "cdecl"):
    RKComplexAbsSquare = _libs["radarkit"].get("RKComplexAbsSquare", "cdecl")
    RKComplexAbsSquare.argtypes = [RKComplex]
    RKComplexAbsSquare.restype = RKFloat

# RKFoundation.h: 108
if _libs["radarkit"].has("RKComplexArrayInConjugate", "cdecl"):
    RKComplexArrayInConjugate = _libs["radarkit"].get("RKComplexArrayInConjugate", "cdecl")
    RKComplexArrayInConjugate.argtypes = [POINTER(RKComplex), c_int]
    RKComplexArrayInConjugate.restype = None

# RKFoundation.h: 109
if _libs["radarkit"].has("RKComplexArrayInPlaceAdd", "cdecl"):
    RKComplexArrayInPlaceAdd = _libs["radarkit"].get("RKComplexArrayInPlaceAdd", "cdecl")
    RKComplexArrayInPlaceAdd.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceAdd.restype = None

# RKFoundation.h: 110
if _libs["radarkit"].has("RKComplexArrayInPlaceSubtract", "cdecl"):
    RKComplexArrayInPlaceSubtract = _libs["radarkit"].get("RKComplexArrayInPlaceSubtract", "cdecl")
    RKComplexArrayInPlaceSubtract.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceSubtract.restype = None

# RKFoundation.h: 111
if _libs["radarkit"].has("RKComplexArrayInPlaceMultiply", "cdecl"):
    RKComplexArrayInPlaceMultiply = _libs["radarkit"].get("RKComplexArrayInPlaceMultiply", "cdecl")
    RKComplexArrayInPlaceMultiply.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceMultiply.restype = None

# RKFoundation.h: 112
if _libs["radarkit"].has("RKComplexArrayInPlaceConjugateMultiply", "cdecl"):
    RKComplexArrayInPlaceConjugateMultiply = _libs["radarkit"].get("RKComplexArrayInPlaceConjugateMultiply", "cdecl")
    RKComplexArrayInPlaceConjugateMultiply.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceConjugateMultiply.restype = None

# RKFoundation.h: 115
if _libs["radarkit"].has("RKFloatArraySum", "cdecl"):
    RKFloatArraySum = _libs["radarkit"].get("RKFloatArraySum", "cdecl")
    RKFloatArraySum.argtypes = [POINTER(RKFloat), c_int]
    RKFloatArraySum.restype = RKFloat

# RKFoundation.h: 116
if _libs["radarkit"].has("RKComplexArraySum", "cdecl"):
    RKComplexArraySum = _libs["radarkit"].get("RKComplexArraySum", "cdecl")
    RKComplexArraySum.argtypes = [POINTER(RKComplex), c_int]
    RKComplexArraySum.restype = RKComplex

# RKFoundation.h: 119
if _libs["radarkit"].has("RKLog", "cdecl"):
    _func = _libs["radarkit"].get("RKLog", "cdecl")
    _restype = c_int
    _errcheck = None
    _argtypes = [String]
    RKLog = _variadic_function(_func,_restype,_argtypes,_errcheck)

# RKFoundation.h: 120
if _libs["radarkit"].has("RKExit", "cdecl"):
    RKExit = _libs["radarkit"].get("RKExit", "cdecl")
    RKExit.argtypes = [c_int]
    RKExit.restype = None

# RKFoundation.h: 123
if _libs["radarkit"].has("RKFileOpen", "cdecl"):
    RKFileOpen = _libs["radarkit"].get("RKFileOpen", "cdecl")
    RKFileOpen.argtypes = [String, String]
    RKFileOpen.restype = POINTER(FILE)

# RKFoundation.h: 124
if _libs["radarkit"].has("RKFileClose", "cdecl"):
    RKFileClose = _libs["radarkit"].get("RKFileClose", "cdecl")
    RKFileClose.argtypes = [POINTER(FILE)]
    RKFileClose.restype = c_int

# RKFoundation.h: 125
if _libs["radarkit"].has("RKFileTell", "cdecl"):
    RKFileTell = _libs["radarkit"].get("RKFileTell", "cdecl")
    RKFileTell.argtypes = [POINTER(FILE)]
    RKFileTell.restype = c_long

# RKFoundation.h: 126
if _libs["radarkit"].has("RKFileGetSize", "cdecl"):
    RKFileGetSize = _libs["radarkit"].get("RKFileGetSize", "cdecl")
    RKFileGetSize.argtypes = [POINTER(FILE)]
    RKFileGetSize.restype = c_size_t

# RKFoundation.h: 127
if _libs["radarkit"].has("RKFileSeek", "cdecl"):
    RKFileSeek = _libs["radarkit"].get("RKFileSeek", "cdecl")
    RKFileSeek.argtypes = [POINTER(FILE), c_long]
    RKFileSeek.restype = c_int

# RKFoundation.h: 128
if _libs["radarkit"].has("RKFileWrite", "cdecl"):
    RKFileWrite = _libs["radarkit"].get("RKFileWrite", "cdecl")
    RKFileWrite.argtypes = [POINTER(None), c_size_t, c_size_t, POINTER(FILE)]
    RKFileWrite.restype = c_size_t

# RKFoundation.h: 129
if _libs["radarkit"].has("RKFileRead", "cdecl"):
    RKFileRead = _libs["radarkit"].get("RKFileRead", "cdecl")
    RKFileRead.argtypes = [POINTER(None), c_size_t, c_size_t, POINTER(FILE)]
    RKFileRead.restype = c_size_t

# RKFoundation.h: 132
if _libs["radarkit"].has("RKSetStatusColor", "cdecl"):
    RKSetStatusColor = _libs["radarkit"].get("RKSetStatusColor", "cdecl")
    RKSetStatusColor.argtypes = [c_bool]
    RKSetStatusColor.restype = None

# RKFoundation.h: 133
if _libs["radarkit"].has("RKSetWantColor", "cdecl"):
    RKSetWantColor = _libs["radarkit"].get("RKSetWantColor", "cdecl")
    RKSetWantColor.argtypes = [c_bool]
    RKSetWantColor.restype = None

# RKFoundation.h: 134
if _libs["radarkit"].has("RKSetWantScreenOutput", "cdecl"):
    RKSetWantScreenOutput = _libs["radarkit"].get("RKSetWantScreenOutput", "cdecl")
    RKSetWantScreenOutput.argtypes = [c_bool]
    RKSetWantScreenOutput.restype = None

# RKFoundation.h: 135
if _libs["radarkit"].has("RKGetWantScreenOutput", "cdecl"):
    RKGetWantScreenOutput = _libs["radarkit"].get("RKGetWantScreenOutput", "cdecl")
    RKGetWantScreenOutput.argtypes = []
    RKGetWantScreenOutput.restype = c_bool

# RKFoundation.h: 136
if _libs["radarkit"].has("RKSetUseDailyLog", "cdecl"):
    RKSetUseDailyLog = _libs["radarkit"].get("RKSetUseDailyLog", "cdecl")
    RKSetUseDailyLog.argtypes = [c_bool]
    RKSetUseDailyLog.restype = None

# RKFoundation.h: 137
if _libs["radarkit"].has("RKSetProgramName", "cdecl"):
    RKSetProgramName = _libs["radarkit"].get("RKSetProgramName", "cdecl")
    RKSetProgramName.argtypes = [String]
    RKSetProgramName.restype = c_int

# RKFoundation.h: 138
if _libs["radarkit"].has("RKSetRootFolder", "cdecl"):
    RKSetRootFolder = _libs["radarkit"].get("RKSetRootFolder", "cdecl")
    RKSetRootFolder.argtypes = [String]
    RKSetRootFolder.restype = c_int

# RKFoundation.h: 139
if _libs["radarkit"].has("RKSetRootDataFolder", "cdecl"):
    RKSetRootDataFolder = _libs["radarkit"].get("RKSetRootDataFolder", "cdecl")
    RKSetRootDataFolder.argtypes = [String]
    RKSetRootDataFolder.restype = c_int

# RKFoundation.h: 140
if _libs["radarkit"].has("RKSetLogfile", "cdecl"):
    RKSetLogfile = _libs["radarkit"].get("RKSetLogfile", "cdecl")
    RKSetLogfile.argtypes = [String]
    RKSetLogfile.restype = c_int

# RKFoundation.h: 141
if _libs["radarkit"].has("RKSetLogfileToDefault", "cdecl"):
    RKSetLogfileToDefault = _libs["radarkit"].get("RKSetLogfileToDefault", "cdecl")
    RKSetLogfileToDefault.argtypes = []
    RKSetLogfileToDefault.restype = c_int

# RKFoundation.h: 143
if _libs["radarkit"].has("RKVersionString", "cdecl"):
    RKVersionString = _libs["radarkit"].get("RKVersionString", "cdecl")
    RKVersionString.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKVersionString.restype = ReturnString
    else:
        RKVersionString.restype = String
        RKVersionString.errcheck = ReturnString

# RKFoundation.h: 144
if _libs["radarkit"].has("RKGuessValueType", "cdecl"):
    RKGuessValueType = _libs["radarkit"].get("RKGuessValueType", "cdecl")
    RKGuessValueType.argtypes = [String]
    RKGuessValueType.restype = RKValueType

# RKFoundation.h: 147
if _libs["radarkit"].has("RKIsFilenameStandard", "cdecl"):
    RKIsFilenameStandard = _libs["radarkit"].get("RKIsFilenameStandard", "cdecl")
    RKIsFilenameStandard.argtypes = [String]
    RKIsFilenameStandard.restype = c_int

# RKFoundation.h: 148
if _libs["radarkit"].has("RKGetSymbolFromFilename", "cdecl"):
    RKGetSymbolFromFilename = _libs["radarkit"].get("RKGetSymbolFromFilename", "cdecl")
    RKGetSymbolFromFilename.argtypes = [String, String]
    RKGetSymbolFromFilename.restype = c_bool

# RKFoundation.h: 149
if _libs["radarkit"].has("RKGetPrefixFromFilename", "cdecl"):
    RKGetPrefixFromFilename = _libs["radarkit"].get("RKGetPrefixFromFilename", "cdecl")
    RKGetPrefixFromFilename.argtypes = [String, String]
    RKGetPrefixFromFilename.restype = c_bool

# RKFoundation.h: 150
if _libs["radarkit"].has("RKListFilesWithSamePrefix", "cdecl"):
    RKListFilesWithSamePrefix = _libs["radarkit"].get("RKListFilesWithSamePrefix", "cdecl")
    RKListFilesWithSamePrefix.argtypes = [String, POINTER(c_char * int(1024))]
    RKListFilesWithSamePrefix.restype = c_int

# RKFoundation.h: 153
if _libs["radarkit"].has("RKShowName", "cdecl"):
    RKShowName = _libs["radarkit"].get("RKShowName", "cdecl")
    RKShowName.argtypes = []
    RKShowName.restype = None

# RKFoundation.h: 154
if _libs["radarkit"].has("RKShowTypeSizes", "cdecl"):
    RKShowTypeSizes = _libs["radarkit"].get("RKShowTypeSizes", "cdecl")
    RKShowTypeSizes.argtypes = []
    RKShowTypeSizes.restype = None

# RKFoundation.h: 155
if _libs["radarkit"].has("RKShowVecFloatLowPrecision", "cdecl"):
    RKShowVecFloatLowPrecision = _libs["radarkit"].get("RKShowVecFloatLowPrecision", "cdecl")
    RKShowVecFloatLowPrecision.argtypes = [String, POINTER(c_float), c_int]
    RKShowVecFloatLowPrecision.restype = None

# RKFoundation.h: 156
if _libs["radarkit"].has("RKShowVecFloat", "cdecl"):
    RKShowVecFloat = _libs["radarkit"].get("RKShowVecFloat", "cdecl")
    RKShowVecFloat.argtypes = [String, POINTER(c_float), c_int]
    RKShowVecFloat.restype = None

# RKFoundation.h: 157
if _libs["radarkit"].has("RKShowVecIQZ", "cdecl"):
    RKShowVecIQZ = _libs["radarkit"].get("RKShowVecIQZ", "cdecl")
    RKShowVecIQZ.argtypes = [String, POINTER(RKIQZ), c_int]
    RKShowVecIQZ.restype = None

# RKFoundation.h: 158
if _libs["radarkit"].has("RKShowVecComplex", "cdecl"):
    RKShowVecComplex = _libs["radarkit"].get("RKShowVecComplex", "cdecl")
    RKShowVecComplex.argtypes = [String, POINTER(RKComplex), c_int]
    RKShowVecComplex.restype = None

# RKFoundation.h: 159
if _libs["radarkit"].has("RKShowArray", "cdecl"):
    RKShowArray = _libs["radarkit"].get("RKShowArray", "cdecl")
    RKShowArray.argtypes = [POINTER(RKFloat), String, c_int, c_int]
    RKShowArray.restype = None

# RKFoundation.h: 160
if _libs["radarkit"].has("RKStringFromValue", "cdecl"):
    RKStringFromValue = _libs["radarkit"].get("RKStringFromValue", "cdecl")
    RKStringFromValue.argtypes = [POINTER(None), RKValueType]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringFromValue.restype = ReturnString
    else:
        RKStringFromValue.restype = String
        RKStringFromValue.errcheck = ReturnString

# RKFoundation.h: 161
if _libs["radarkit"].has("RKVariableInString", "cdecl"):
    RKVariableInString = _libs["radarkit"].get("RKVariableInString", "cdecl")
    RKVariableInString.argtypes = [String, POINTER(None), RKValueType]
    if sizeof(c_int) == sizeof(c_void_p):
        RKVariableInString.restype = ReturnString
    else:
        RKVariableInString.restype = String
        RKVariableInString.errcheck = ReturnString

# RKFoundation.h: 162
if _libs["radarkit"].has("RKPrettyStringSizeEstimate", "cdecl"):
    RKPrettyStringSizeEstimate = _libs["radarkit"].get("RKPrettyStringSizeEstimate", "cdecl")
    RKPrettyStringSizeEstimate.argtypes = [String]
    RKPrettyStringSizeEstimate.restype = c_size_t

# RKFoundation.h: 163
if _libs["radarkit"].has("RKPrettyStringFromKeyValueString", "cdecl"):
    RKPrettyStringFromKeyValueString = _libs["radarkit"].get("RKPrettyStringFromKeyValueString", "cdecl")
    RKPrettyStringFromKeyValueString.argtypes = [String, String]
    RKPrettyStringFromKeyValueString.restype = c_size_t

# RKFoundation.h: 166
if _libs["radarkit"].has("RKMalloc", "cdecl"):
    RKMalloc = _libs["radarkit"].get("RKMalloc", "cdecl")
    RKMalloc.argtypes = [uint32_t]
    RKMalloc.restype = POINTER(c_ubyte)
    RKMalloc.errcheck = lambda v,*a : cast(v, c_void_p)

# RKFoundation.h: 167
if _libs["radarkit"].has("RKZeroOutFloat", "cdecl"):
    RKZeroOutFloat = _libs["radarkit"].get("RKZeroOutFloat", "cdecl")
    RKZeroOutFloat.argtypes = [POINTER(RKFloat), uint32_t]
    RKZeroOutFloat.restype = None

# RKFoundation.h: 168
if _libs["radarkit"].has("RKZeroOutIQZ", "cdecl"):
    RKZeroOutIQZ = _libs["radarkit"].get("RKZeroOutIQZ", "cdecl")
    RKZeroOutIQZ.argtypes = [POINTER(RKIQZ), uint32_t]
    RKZeroOutIQZ.restype = None

# RKFoundation.h: 169
if _libs["radarkit"].has("RKZeroTailFloat", "cdecl"):
    RKZeroTailFloat = _libs["radarkit"].get("RKZeroTailFloat", "cdecl")
    RKZeroTailFloat.argtypes = [POINTER(RKFloat), uint32_t, uint32_t]
    RKZeroTailFloat.restype = None

# RKFoundation.h: 170
if _libs["radarkit"].has("RKZeroTailIQZ", "cdecl"):
    RKZeroTailIQZ = _libs["radarkit"].get("RKZeroTailIQZ", "cdecl")
    RKZeroTailIQZ.argtypes = [POINTER(RKIQZ), uint32_t, uint32_t]
    RKZeroTailIQZ.restype = None

# RKFoundation.h: 173
if _libs["radarkit"].has("RKPulseBufferAlloc", "cdecl"):
    RKPulseBufferAlloc = _libs["radarkit"].get("RKPulseBufferAlloc", "cdecl")
    RKPulseBufferAlloc.argtypes = [POINTER(RKBuffer), uint32_t, uint32_t]
    RKPulseBufferAlloc.restype = c_size_t

# RKFoundation.h: 174
if _libs["radarkit"].has("RKPulseBufferFree", "cdecl"):
    RKPulseBufferFree = _libs["radarkit"].get("RKPulseBufferFree", "cdecl")
    RKPulseBufferFree.argtypes = [RKBuffer]
    RKPulseBufferFree.restype = None

# RKFoundation.h: 175
if _libs["radarkit"].has("RKGetPulseFromBuffer", "cdecl"):
    RKGetPulseFromBuffer = _libs["radarkit"].get("RKGetPulseFromBuffer", "cdecl")
    RKGetPulseFromBuffer.argtypes = [RKBuffer, uint32_t]
    RKGetPulseFromBuffer.restype = POINTER(RKPulse)

# RKFoundation.h: 176
if _libs["radarkit"].has("RKGetInt16CDataFromPulse", "cdecl"):
    RKGetInt16CDataFromPulse = _libs["radarkit"].get("RKGetInt16CDataFromPulse", "cdecl")
    RKGetInt16CDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetInt16CDataFromPulse.restype = POINTER(RKInt16C)

# RKFoundation.h: 177
if _libs["radarkit"].has("RKGetComplexDataFromPulse", "cdecl"):
    RKGetComplexDataFromPulse = _libs["radarkit"].get("RKGetComplexDataFromPulse", "cdecl")
    RKGetComplexDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetComplexDataFromPulse.restype = POINTER(RKComplex)

# RKFoundation.h: 178
if _libs["radarkit"].has("RKGetSplitComplexDataFromPulse", "cdecl"):
    RKGetSplitComplexDataFromPulse = _libs["radarkit"].get("RKGetSplitComplexDataFromPulse", "cdecl")
    RKGetSplitComplexDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetSplitComplexDataFromPulse.restype = RKIQZ

# RKFoundation.h: 179
if _libs["radarkit"].has("RKClearPulseBuffer", "cdecl"):
    RKClearPulseBuffer = _libs["radarkit"].get("RKClearPulseBuffer", "cdecl")
    RKClearPulseBuffer.argtypes = [RKBuffer, uint32_t]
    RKClearPulseBuffer.restype = c_int

# RKFoundation.h: 180
if _libs["radarkit"].has("RKReadPulseFromFileReference", "cdecl"):
    RKReadPulseFromFileReference = _libs["radarkit"].get("RKReadPulseFromFileReference", "cdecl")
    RKReadPulseFromFileReference.argtypes = [POINTER(RKPulse), POINTER(RKFileHeader), POINTER(FILE)]
    RKReadPulseFromFileReference.restype = c_int

# RKFoundation.h: 181
if _libs["radarkit"].has("RKGetVacantPulseFromBuffer", "cdecl"):
    RKGetVacantPulseFromBuffer = _libs["radarkit"].get("RKGetVacantPulseFromBuffer", "cdecl")
    RKGetVacantPulseFromBuffer.argtypes = [RKBuffer, POINTER(uint32_t), uint32_t]
    RKGetVacantPulseFromBuffer.restype = POINTER(RKPulse)

# RKFoundation.h: 182
if _libs["radarkit"].has("RKPulseBufferAllocCopyFromBuffer", "cdecl"):
    RKPulseBufferAllocCopyFromBuffer = _libs["radarkit"].get("RKPulseBufferAllocCopyFromBuffer", "cdecl")
    RKPulseBufferAllocCopyFromBuffer.argtypes = [RKBuffer, uint32_t, uint32_t, uint32_t]
    RKPulseBufferAllocCopyFromBuffer.restype = RKBuffer

# RKFoundation.h: 183
if _libs["radarkit"].has("RKPulseDuplicateSplitComplex", "cdecl"):
    RKPulseDuplicateSplitComplex = _libs["radarkit"].get("RKPulseDuplicateSplitComplex", "cdecl")
    RKPulseDuplicateSplitComplex.argtypes = [POINTER(RKPulse)]
    RKPulseDuplicateSplitComplex.restype = None

# RKFoundation.h: 186
if _libs["radarkit"].has("RKRayBufferAlloc", "cdecl"):
    RKRayBufferAlloc = _libs["radarkit"].get("RKRayBufferAlloc", "cdecl")
    RKRayBufferAlloc.argtypes = [POINTER(RKBuffer), uint32_t, uint32_t]
    RKRayBufferAlloc.restype = c_size_t

# RKFoundation.h: 187
if _libs["radarkit"].has("RKRayBufferFree", "cdecl"):
    RKRayBufferFree = _libs["radarkit"].get("RKRayBufferFree", "cdecl")
    RKRayBufferFree.argtypes = [RKBuffer]
    RKRayBufferFree.restype = None

# RKFoundation.h: 188
if _libs["radarkit"].has("RKGetRayFromBuffer", "cdecl"):
    RKGetRayFromBuffer = _libs["radarkit"].get("RKGetRayFromBuffer", "cdecl")
    RKGetRayFromBuffer.argtypes = [RKBuffer, uint32_t]
    RKGetRayFromBuffer.restype = POINTER(RKRay)

# RKFoundation.h: 189
if _libs["radarkit"].has("RKGetUInt8DataFromRay", "cdecl"):
    RKGetUInt8DataFromRay = _libs["radarkit"].get("RKGetUInt8DataFromRay", "cdecl")
    RKGetUInt8DataFromRay.argtypes = [POINTER(RKRay), RKProductIndex]
    RKGetUInt8DataFromRay.restype = POINTER(uint8_t)

# RKFoundation.h: 190
if _libs["radarkit"].has("RKGetFloatDataFromRay", "cdecl"):
    RKGetFloatDataFromRay = _libs["radarkit"].get("RKGetFloatDataFromRay", "cdecl")
    RKGetFloatDataFromRay.argtypes = [POINTER(RKRay), RKProductIndex]
    RKGetFloatDataFromRay.restype = POINTER(c_float)

# RKFoundation.h: 191
if _libs["radarkit"].has("RKClearRayBuffer", "cdecl"):
    RKClearRayBuffer = _libs["radarkit"].get("RKClearRayBuffer", "cdecl")
    RKClearRayBuffer.argtypes = [RKBuffer, uint32_t]
    RKClearRayBuffer.restype = c_int

# RKFoundation.h: 192
if _libs["radarkit"].has("RKGetVacantRayFromBuffer", "cdecl"):
    RKGetVacantRayFromBuffer = _libs["radarkit"].get("RKGetVacantRayFromBuffer", "cdecl")
    RKGetVacantRayFromBuffer.argtypes = [RKBuffer, POINTER(uint32_t), uint32_t]
    RKGetVacantRayFromBuffer.restype = POINTER(RKRay)

# RKFoundation.h: 195
if _libs["radarkit"].has("RKFileMonitorInit", "cdecl"):
    RKFileMonitorInit = _libs["radarkit"].get("RKFileMonitorInit", "cdecl")
    RKFileMonitorInit.argtypes = [String, CFUNCTYPE(UNCHECKED(None), POINTER(None)), POINTER(None)]
    RKFileMonitorInit.restype = POINTER(RKFileMonitor)

# RKFoundation.h: 196
if _libs["radarkit"].has("RKFileMonitorFree", "cdecl"):
    RKFileMonitorFree = _libs["radarkit"].get("RKFileMonitorFree", "cdecl")
    RKFileMonitorFree.argtypes = [POINTER(RKFileMonitor)]
    RKFileMonitorFree.restype = c_int

# RKFoundation.h: 199
if _libs["radarkit"].has("RKStreamFromString", "cdecl"):
    RKStreamFromString = _libs["radarkit"].get("RKStreamFromString", "cdecl")
    RKStreamFromString.argtypes = [String]
    RKStreamFromString.restype = RKStream

# RKFoundation.h: 200
if _libs["radarkit"].has("RKStringOfStream", "cdecl"):
    RKStringOfStream = _libs["radarkit"].get("RKStringOfStream", "cdecl")
    RKStringOfStream.argtypes = [RKStream]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringOfStream.restype = ReturnString
    else:
        RKStringOfStream.restype = String
        RKStringOfStream.errcheck = ReturnString

# RKFoundation.h: 201
if _libs["radarkit"].has("RKStringFromStream", "cdecl"):
    RKStringFromStream = _libs["radarkit"].get("RKStringFromStream", "cdecl")
    RKStringFromStream.argtypes = [String, RKStream]
    RKStringFromStream.restype = c_int

# RKFoundation.h: 203
if _libs["radarkit"].has("RKGetNextProductDescription", "cdecl"):
    RKGetNextProductDescription = _libs["radarkit"].get("RKGetNextProductDescription", "cdecl")
    RKGetNextProductDescription.argtypes = [POINTER(RKProductList)]
    RKGetNextProductDescription.restype = RKProductDesc

# RKFoundation.h: 206
if _libs["radarkit"].has("RKParseCommaDelimitedValues", "cdecl"):
    RKParseCommaDelimitedValues = _libs["radarkit"].get("RKParseCommaDelimitedValues", "cdecl")
    RKParseCommaDelimitedValues.argtypes = [POINTER(None), RKValueType, c_size_t, String]
    RKParseCommaDelimitedValues.restype = c_size_t

# RKFoundation.h: 207
if _libs["radarkit"].has("RKParseNumericArray", "cdecl"):
    RKParseNumericArray = _libs["radarkit"].get("RKParseNumericArray", "cdecl")
    RKParseNumericArray.argtypes = [POINTER(None), RKValueType, c_size_t, String]
    RKParseNumericArray.restype = c_size_t

# RKFoundation.h: 208
if _libs["radarkit"].has("RKParseQuotedStrings", "cdecl"):
    _func = _libs["radarkit"].get("RKParseQuotedStrings", "cdecl")
    _restype = None
    _errcheck = None
    _argtypes = [String]
    RKParseQuotedStrings = _variadic_function(_func,_restype,_argtypes,_errcheck)

# RKFoundation.h: 209
if _libs["radarkit"].has("RKMakeJSONStringFromControls", "cdecl"):
    RKMakeJSONStringFromControls = _libs["radarkit"].get("RKMakeJSONStringFromControls", "cdecl")
    RKMakeJSONStringFromControls.argtypes = [String, POINTER(RKControl), uint32_t]
    RKMakeJSONStringFromControls.restype = None

# RKFoundation.h: 210
if _libs["radarkit"].has("RKValueToEnum", "cdecl"):
    RKValueToEnum = _libs["radarkit"].get("RKValueToEnum", "cdecl")
    RKValueToEnum.argtypes = [RKConst, RKConst, RKConst, RKConst, RKConst, RKConst, RKConst]
    RKValueToEnum.restype = RKStatusEnum

# RKFoundation.h: 211
if _libs["radarkit"].has("RKStatusFromTemperatureForCE", "cdecl"):
    RKStatusFromTemperatureForCE = _libs["radarkit"].get("RKStatusFromTemperatureForCE", "cdecl")
    RKStatusFromTemperatureForCE.argtypes = [RKConst]
    RKStatusFromTemperatureForCE.restype = RKStatusEnum

# RKFoundation.h: 212
if _libs["radarkit"].has("RKStatusFromTemperatureForIE", "cdecl"):
    RKStatusFromTemperatureForIE = _libs["radarkit"].get("RKStatusFromTemperatureForIE", "cdecl")
    RKStatusFromTemperatureForIE.argtypes = [RKConst]
    RKStatusFromTemperatureForIE.restype = RKStatusEnum

# RKFoundation.h: 213
if _libs["radarkit"].has("RKStatusFromTemperatureForComputers", "cdecl"):
    RKStatusFromTemperatureForComputers = _libs["radarkit"].get("RKStatusFromTemperatureForComputers", "cdecl")
    RKStatusFromTemperatureForComputers.argtypes = [RKConst]
    RKStatusFromTemperatureForComputers.restype = RKStatusEnum

# RKFoundation.h: 214
if _libs["radarkit"].has("RKFindCondition", "cdecl"):
    RKFindCondition = _libs["radarkit"].get("RKFindCondition", "cdecl")
    RKFindCondition.argtypes = [String, RKStatusEnum, c_bool, String, String]
    RKFindCondition.restype = c_bool

# RKFoundation.h: 215
if _libs["radarkit"].has("RKAnyCritical", "cdecl"):
    RKAnyCritical = _libs["radarkit"].get("RKAnyCritical", "cdecl")
    RKAnyCritical.argtypes = [String, c_bool, String, String]
    RKAnyCritical.restype = c_bool

# RKFoundation.h: 216
if _libs["radarkit"].has("RKParseProductDescription", "cdecl"):
    RKParseProductDescription = _libs["radarkit"].get("RKParseProductDescription", "cdecl")
    RKParseProductDescription.argtypes = [POINTER(RKProductDesc), String]
    RKParseProductDescription.restype = c_int

# RKFoundation.h: 217
if _libs["radarkit"].has("RKProductIdFromString", "cdecl"):
    RKProductIdFromString = _libs["radarkit"].get("RKProductIdFromString", "cdecl")
    RKProductIdFromString.argtypes = [String]
    RKProductIdFromString.restype = RKProductId

# RKFoundation.h: 218
if _libs["radarkit"].has("RKIdentifierFromString", "cdecl"):
    RKIdentifierFromString = _libs["radarkit"].get("RKIdentifierFromString", "cdecl")
    RKIdentifierFromString.argtypes = [String]
    RKIdentifierFromString.restype = RKIdentifier

# RKFoundation.h: 221
if _libs["radarkit"].has("RKSimpleEngineFree", "cdecl"):
    RKSimpleEngineFree = _libs["radarkit"].get("RKSimpleEngineFree", "cdecl")
    RKSimpleEngineFree.argtypes = [POINTER(RKSimpleEngine)]
    RKSimpleEngineFree.restype = c_int

# RKFoundation.h: 224
if _libs["radarkit"].has("RKCommandQueueInit", "cdecl"):
    RKCommandQueueInit = _libs["radarkit"].get("RKCommandQueueInit", "cdecl")
    RKCommandQueueInit.argtypes = [uint16_t, c_bool]
    RKCommandQueueInit.restype = POINTER(RKCommandQueue)

# RKFoundation.h: 225
if _libs["radarkit"].has("RKCommandQueuePop", "cdecl"):
    RKCommandQueuePop = _libs["radarkit"].get("RKCommandQueuePop", "cdecl")
    RKCommandQueuePop.argtypes = [POINTER(RKCommandQueue)]
    RKCommandQueuePop.restype = POINTER(RKCommand)

# RKFoundation.h: 226
if _libs["radarkit"].has("RKCommandQueuePush", "cdecl"):
    RKCommandQueuePush = _libs["radarkit"].get("RKCommandQueuePush", "cdecl")
    RKCommandQueuePush.argtypes = [POINTER(RKCommandQueue), POINTER(RKCommand)]
    RKCommandQueuePush.restype = c_int

# RKFoundation.h: 227
if _libs["radarkit"].has("RKCommandQueueFree", "cdecl"):
    RKCommandQueueFree = _libs["radarkit"].get("RKCommandQueueFree", "cdecl")
    RKCommandQueueFree.argtypes = [POINTER(RKCommandQueue)]
    RKCommandQueueFree.restype = c_int

# RKFoundation.h: 230
if _libs["radarkit"].has("RKPedestalActionString", "cdecl"):
    RKPedestalActionString = _libs["radarkit"].get("RKPedestalActionString", "cdecl")
    RKPedestalActionString.argtypes = [POINTER(RKScanAction)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPedestalActionString.restype = ReturnString
    else:
        RKPedestalActionString.restype = String
        RKPedestalActionString.errcheck = ReturnString

# RKConfig.h: 14
if _libs["radarkit"].has("RKConfigBufferAlloc", "cdecl"):
    RKConfigBufferAlloc = _libs["radarkit"].get("RKConfigBufferAlloc", "cdecl")
    RKConfigBufferAlloc.argtypes = [POINTER(POINTER(RKConfig)), uint32_t]
    RKConfigBufferAlloc.restype = c_size_t

# RKConfig.h: 15
if _libs["radarkit"].has("RKConfigBufferFree", "cdecl"):
    RKConfigBufferFree = _libs["radarkit"].get("RKConfigBufferFree", "cdecl")
    RKConfigBufferFree.argtypes = [POINTER(RKConfig)]
    RKConfigBufferFree.restype = None

# RKConfig.h: 17
if _libs["radarkit"].has("RKConfigAdvanceEllipsis", "cdecl"):
    _func = _libs["radarkit"].get("RKConfigAdvanceEllipsis", "cdecl")
    _restype = None
    _errcheck = None
    _argtypes = [POINTER(RKConfig), POINTER(uint32_t), uint32_t]
    RKConfigAdvanceEllipsis = _variadic_function(_func,_restype,_argtypes,_errcheck)

# RKConfig.h: 18
if _libs["radarkit"].has("RKConfigAdvance", "cdecl"):
    RKConfigAdvance = _libs["radarkit"].get("RKConfigAdvance", "cdecl")
    RKConfigAdvance.argtypes = [POINTER(RKConfig), POINTER(uint32_t), uint32_t, c_void_p]
    RKConfigAdvance.restype = None

# RKConfig.h: 21
if _libs["radarkit"].has("RKConfigWithId", "cdecl"):
    RKConfigWithId = _libs["radarkit"].get("RKConfigWithId", "cdecl")
    RKConfigWithId.argtypes = [POINTER(RKConfig), uint32_t, uint64_t]
    RKConfigWithId.restype = POINTER(RKConfig)

fftwf_complex = c_float * int(2)# /usr/include/fftw3.h: 466

# /usr/include/fftw3.h: 466
class struct_fftwf_plan_s(Structure):
    pass

fftwf_plan = POINTER(struct_fftwf_plan_s)# /usr/include/fftw3.h: 466

RKWindowType = c_int# headers/RadarKit/RKWindow.h: 14

# RKDSP.h: 25
class struct_rk_fft_resource(Structure):
    pass

struct_rk_fft_resource.__slots__ = [
    'size',
    'count',
    'forwardInPlace',
    'forwardOutPlace',
    'backwardInPlace',
    'backwardOutPlace',
]
struct_rk_fft_resource._fields_ = [
    ('size', c_uint),
    ('count', c_uint),
    ('forwardInPlace', fftwf_plan),
    ('forwardOutPlace', fftwf_plan),
    ('backwardInPlace', fftwf_plan),
    ('backwardOutPlace', fftwf_plan),
]

RKFFTResource = struct_rk_fft_resource# RKDSP.h: 25

# RKDSP.h: 34
class struct_rk_fft_module(Structure):
    pass

struct_rk_fft_module.__slots__ = [
    'name',
    'verbose',
    'exportWisdom',
    'count',
    'wisdomFile',
    'plans',
]
struct_rk_fft_module._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('exportWisdom', c_bool),
    ('count', c_uint),
    ('wisdomFile', c_char * int(1024)),
    ('plans', POINTER(RKFFTResource)),
]

RKFFTModule = struct_rk_fft_module# RKDSP.h: 34

# RKDSP.h: 40
class struct_rk_gaussian(Structure):
    pass

struct_rk_gaussian.__slots__ = [
    'A',
    'mu',
    'sigma',
]
struct_rk_gaussian._fields_ = [
    ('A', RKFloat),
    ('mu', RKFloat),
    ('sigma', RKFloat),
]

RKGaussian = struct_rk_gaussian# RKDSP.h: 40

# RKDSP.h: 46
if _libs["radarkit"].has("RKGetSignedMinorSectorInDegrees", "cdecl"):
    RKGetSignedMinorSectorInDegrees = _libs["radarkit"].get("RKGetSignedMinorSectorInDegrees", "cdecl")
    RKGetSignedMinorSectorInDegrees.argtypes = [c_float, c_float]
    RKGetSignedMinorSectorInDegrees.restype = c_float

# RKDSP.h: 47
if _libs["radarkit"].has("RKGetMinorSectorInDegrees", "cdecl"):
    RKGetMinorSectorInDegrees = _libs["radarkit"].get("RKGetMinorSectorInDegrees", "cdecl")
    RKGetMinorSectorInDegrees.argtypes = [c_float, c_float]
    RKGetMinorSectorInDegrees.restype = c_float

# RKDSP.h: 48
if _libs["radarkit"].has("RKInterpolatePositiveAngles", "cdecl"):
    RKInterpolatePositiveAngles = _libs["radarkit"].get("RKInterpolatePositiveAngles", "cdecl")
    RKInterpolatePositiveAngles.argtypes = [c_float, c_float, c_float]
    RKInterpolatePositiveAngles.restype = c_float

# RKDSP.h: 49
if _libs["radarkit"].has("RKInterpolateAngles", "cdecl"):
    RKInterpolateAngles = _libs["radarkit"].get("RKInterpolateAngles", "cdecl")
    RKInterpolateAngles.argtypes = [c_float, c_float, c_float]
    RKInterpolateAngles.restype = c_float

# RKDSP.h: 51
if _libs["radarkit"].has("RKMeasureNoiseFromPulse", "cdecl"):
    RKMeasureNoiseFromPulse = _libs["radarkit"].get("RKMeasureNoiseFromPulse", "cdecl")
    RKMeasureNoiseFromPulse.argtypes = [POINTER(RKFloat), POINTER(RKPulse), c_int]
    RKMeasureNoiseFromPulse.restype = c_int

# RKDSP.h: 52
if _libs["radarkit"].has("RKBestStrideOfHopsV1", "cdecl"):
    RKBestStrideOfHopsV1 = _libs["radarkit"].get("RKBestStrideOfHopsV1", "cdecl")
    RKBestStrideOfHopsV1.argtypes = [c_int, c_bool]
    RKBestStrideOfHopsV1.restype = c_int

# RKDSP.h: 53
if _libs["radarkit"].has("RKBestStrideOfHops", "cdecl"):
    RKBestStrideOfHops = _libs["radarkit"].get("RKBestStrideOfHops", "cdecl")
    RKBestStrideOfHops.argtypes = [c_int, c_bool]
    RKBestStrideOfHops.restype = c_int

# RKDSP.h: 55
if _libs["radarkit"].has("RKHilbertTransform", "cdecl"):
    RKHilbertTransform = _libs["radarkit"].get("RKHilbertTransform", "cdecl")
    RKHilbertTransform.argtypes = [POINTER(RKFloat), POINTER(RKComplex), c_int]
    RKHilbertTransform.restype = None

# RKDSP.h: 57
if _libs["radarkit"].has("RKFasterSineCosine", "cdecl"):
    RKFasterSineCosine = _libs["radarkit"].get("RKFasterSineCosine", "cdecl")
    RKFasterSineCosine.argtypes = [c_float, POINTER(c_float), POINTER(c_float)]
    RKFasterSineCosine.restype = None

# RKDSP.h: 58
if _libs["radarkit"].has("RKFastSineCosine", "cdecl"):
    RKFastSineCosine = _libs["radarkit"].get("RKFastSineCosine", "cdecl")
    RKFastSineCosine.argtypes = [c_float, POINTER(c_float), POINTER(c_float)]
    RKFastSineCosine.restype = None

# RKDSP.h: 64
if _libs["radarkit"].has("RKGetFilterCoefficients", "cdecl"):
    RKGetFilterCoefficients = _libs["radarkit"].get("RKGetFilterCoefficients", "cdecl")
    RKGetFilterCoefficients.argtypes = [POINTER(RKIIRFilter), RKFilterType]
    RKGetFilterCoefficients.restype = None

# RKDSP.h: 70
if _libs["radarkit"].has("RKFFTModuleInit", "cdecl"):
    RKFFTModuleInit = _libs["radarkit"].get("RKFFTModuleInit", "cdecl")
    RKFFTModuleInit.argtypes = [uint32_t, c_int]
    RKFFTModuleInit.restype = POINTER(RKFFTModule)

# RKDSP.h: 71
if _libs["radarkit"].has("RKFFTModuleFree", "cdecl"):
    RKFFTModuleFree = _libs["radarkit"].get("RKFFTModuleFree", "cdecl")
    RKFFTModuleFree.argtypes = [POINTER(RKFFTModule)]
    RKFFTModuleFree.restype = None

# RKDSP.h: 77
if _libs["radarkit"].has("RKSGFit", "cdecl"):
    RKSGFit = _libs["radarkit"].get("RKSGFit", "cdecl")
    RKSGFit.argtypes = [POINTER(RKFloat), POINTER(RKComplex), c_int]
    RKSGFit.restype = RKGaussian

# RKDSP.h: 83
if _libs["radarkit"].has("RKSingle2Double", "cdecl"):
    RKSingle2Double = _libs["radarkit"].get("RKSingle2Double", "cdecl")
    RKSingle2Double.argtypes = [RKWordFloat32]
    RKSingle2Double.restype = RKWordFloat64

# RKDSP.h: 84
if _libs["radarkit"].has("RKHalf2Single", "cdecl"):
    RKHalf2Single = _libs["radarkit"].get("RKHalf2Single", "cdecl")
    RKHalf2Single.argtypes = [RKWordFloat16]
    RKHalf2Single.restype = RKWordFloat32

# RKDSP.h: 89
if _libs["radarkit"].has("RKBitReverse32", "cdecl"):
    RKBitReverse32 = _libs["radarkit"].get("RKBitReverse32", "cdecl")
    RKBitReverse32.argtypes = [uint32_t]
    RKBitReverse32.restype = uint32_t

# RKDSP.h: 90
for _lib in _libs.values():
    if not _lib.has("RKBitReverse64", "cdecl"):
        continue
    RKBitReverse64 = _lib.get("RKBitReverse64", "cdecl")
    RKBitReverse64.argtypes = [uint16_t]
    RKBitReverse64.restype = uint16_t
    break

# RKDSP.h: 91
if _libs["radarkit"].has("RKBitReverse8", "cdecl"):
    RKBitReverse8 = _libs["radarkit"].get("RKBitReverse8", "cdecl")
    RKBitReverse8.argtypes = [uint8_t]
    RKBitReverse8.restype = uint8_t

# RKDSP.h: 96
if _libs["radarkit"].has("RKShowWordFloat16", "cdecl"):
    RKShowWordFloat16 = _libs["radarkit"].get("RKShowWordFloat16", "cdecl")
    RKShowWordFloat16.argtypes = [RKWordFloat16, c_float]
    RKShowWordFloat16.restype = None

# RKDSP.h: 97
if _libs["radarkit"].has("RKShowWordFloat32", "cdecl"):
    RKShowWordFloat32 = _libs["radarkit"].get("RKShowWordFloat32", "cdecl")
    RKShowWordFloat32.argtypes = [RKWordFloat32, c_float]
    RKShowWordFloat32.restype = None

# RKDSP.h: 98
if _libs["radarkit"].has("RKShowWordFloat64", "cdecl"):
    RKShowWordFloat64 = _libs["radarkit"].get("RKShowWordFloat64", "cdecl")
    RKShowWordFloat64.argtypes = [RKWordFloat64, c_float]
    RKShowWordFloat64.restype = None

RKCellMask = c_int8# headers/RadarKit/RKScratch.h: 15

enum_anon_167 = c_int# headers/RadarKit/RKScratch.h: 16

RKCellMaskNull = 0# headers/RadarKit/RKScratch.h: 16

RKCellMaskKeepH = 1# headers/RadarKit/RKScratch.h: 16

RKCellMaskKeepV = (1 << 1)# headers/RadarKit/RKScratch.h: 16

RKCellMaskKeepBoth = (RKCellMaskKeepH | RKCellMaskKeepV)# headers/RadarKit/RKScratch.h: 16

# headers/RadarKit/RKScratch.h: 52
class struct_rk_compression_scratch(Structure):
    pass

struct_rk_compression_scratch.__slots__ = [
    'name',
    'verbose',
    'capacity',
    'pulse',
    'filter',
    'filterAnchor',
    'fftModule',
    'inBuffer',
    'outBuffer',
    'zi',
    'zo',
    'user1',
    'user2',
    'user3',
    'user4',
    'userComplex1',
    'userComplex2',
    'userComplex3',
    'userComplex4',
    'config',
    'waveformGroupdId',
    'waveformFilterId',
    'planIndex',
    'userResource',
]
struct_rk_compression_scratch._fields_ = [
    ('name', RKName),
    ('verbose', uint8_t),
    ('capacity', uint32_t),
    ('pulse', POINTER(RKPulse)),
    ('filter', POINTER(RKComplex)),
    ('filterAnchor', POINTER(RKFilterAnchor)),
    ('fftModule', POINTER(RKFFTModule)),
    ('inBuffer', POINTER(fftwf_complex)),
    ('outBuffer', POINTER(fftwf_complex)),
    ('zi', POINTER(RKIQZ)),
    ('zo', POINTER(RKIQZ)),
    ('user1', POINTER(RKFloat)),
    ('user2', POINTER(RKFloat)),
    ('user3', POINTER(RKFloat)),
    ('user4', POINTER(RKFloat)),
    ('userComplex1', POINTER(RKComplex)),
    ('userComplex2', POINTER(RKComplex)),
    ('userComplex3', POINTER(RKComplex)),
    ('userComplex4', POINTER(RKComplex)),
    ('config', POINTER(RKConfig)),
    ('waveformGroupdId', uint16_t),
    ('waveformFilterId', uint16_t),
    ('planIndex', uint16_t),
    ('userResource', RKUserResource),
]

RKCompressionScratch = struct_rk_compression_scratch# headers/RadarKit/RKScratch.h: 52

# headers/RadarKit/RKScratch.h: 112
class struct_rk_moment_scratch(Structure):
    pass

struct_rk_moment_scratch.__slots__ = [
    'name',
    'verbose',
    'capacity',
    'userLagChoice',
    'gateCount',
    'gateSizeMeters',
    'samplingAdjustment',
    'mX',
    'vX',
    'R',
    'C',
    'sC',
    'ts',
    'RX',
    'CX',
    'aR',
    'aC',
    'aRX',
    'aCX',
    'gC',
    'noise',
    'velocityFactor',
    'widthFactor',
    'KDPFactor',
    'dcal',
    'pcal',
    'S2Z',
    'S',
    'Z',
    'V',
    'W',
    'Q',
    'L',
    'RhoXP',
    'PhiXP',
    'SNR',
    'ZDR',
    'PhiDP',
    'RhoHV',
    'KDP',
    'user1',
    'user2',
    'user3',
    'user4',
    'mask',
    'fftModule',
    'inBuffer',
    'outBuffer',
    'fS',
    'fC',
    'fftOrder',
    'config',
    'calculatedMoments',
    'calculatedProducts',
]
struct_rk_moment_scratch._fields_ = [
    ('name', RKName),
    ('verbose', uint8_t),
    ('capacity', uint32_t),
    ('userLagChoice', uint8_t),
    ('gateCount', uint16_t),
    ('gateSizeMeters', RKFloat),
    ('samplingAdjustment', RKFloat),
    ('mX', RKIQZ * int(2)),
    ('vX', RKIQZ * int(2)),
    ('R', (RKIQZ * int(5)) * int(2)),
    ('C', RKIQZ * int(((2 * 5) - 1))),
    ('sC', RKIQZ),
    ('ts', RKIQZ),
    ('RX', (RKIQZ * int(5)) * int(2)),
    ('CX', (RKIQZ * int(((2 * 5) - 1))) * int(2)),
    ('aR', (POINTER(RKFloat) * int(5)) * int(2)),
    ('aC', POINTER(RKFloat) * int(((2 * 5) - 1))),
    ('aRX', (POINTER(RKFloat) * int(5)) * int(2)),
    ('aCX', (POINTER(RKFloat) * int(((2 * 5) - 1))) * int(2)),
    ('gC', POINTER(RKFloat)),
    ('noise', RKFloat * int(2)),
    ('velocityFactor', RKFloat),
    ('widthFactor', RKFloat),
    ('KDPFactor', RKFloat),
    ('dcal', POINTER(RKFloat)),
    ('pcal', POINTER(RKFloat)),
    ('S2Z', POINTER(RKFloat) * int(2)),
    ('S', POINTER(RKFloat) * int(2)),
    ('Z', POINTER(RKFloat) * int(2)),
    ('V', POINTER(RKFloat) * int(2)),
    ('W', POINTER(RKFloat) * int(2)),
    ('Q', POINTER(RKFloat) * int(2)),
    ('L', POINTER(RKFloat) * int(2)),
    ('RhoXP', POINTER(RKFloat) * int(2)),
    ('PhiXP', POINTER(RKFloat) * int(2)),
    ('SNR', POINTER(RKFloat) * int(2)),
    ('ZDR', POINTER(RKFloat)),
    ('PhiDP', POINTER(RKFloat)),
    ('RhoHV', POINTER(RKFloat)),
    ('KDP', POINTER(RKFloat)),
    ('user1', POINTER(RKFloat)),
    ('user2', POINTER(RKFloat)),
    ('user3', POINTER(RKFloat)),
    ('user4', POINTER(RKFloat)),
    ('mask', POINTER(uint8_t)),
    ('fftModule', POINTER(RKFFTModule)),
    ('inBuffer', POINTER(POINTER(fftwf_complex))),
    ('outBuffer', POINTER(POINTER(fftwf_complex))),
    ('fS', POINTER(POINTER(fftwf_complex)) * int(2)),
    ('fC', POINTER(POINTER(fftwf_complex))),
    ('fftOrder', c_int8),
    ('config', POINTER(RKConfig)),
    ('calculatedMoments', RKMomentList),
    ('calculatedProducts', RKProductList),
]

RKMomentScratch = struct_rk_moment_scratch# headers/RadarKit/RKScratch.h: 112

# headers/RadarKit/RKScratch.h: 114
if _libs["radarkit"].has("RKCompressionScratchAlloc", "cdecl"):
    RKCompressionScratchAlloc = _libs["radarkit"].get("RKCompressionScratchAlloc", "cdecl")
    RKCompressionScratchAlloc.argtypes = [POINTER(POINTER(RKCompressionScratch)), uint32_t, uint8_t, String]
    RKCompressionScratchAlloc.restype = c_size_t

# headers/RadarKit/RKScratch.h: 115
if _libs["radarkit"].has("RKCompressionScratchFree", "cdecl"):
    RKCompressionScratchFree = _libs["radarkit"].get("RKCompressionScratchFree", "cdecl")
    RKCompressionScratchFree.argtypes = [POINTER(RKCompressionScratch)]
    RKCompressionScratchFree.restype = None

# headers/RadarKit/RKScratch.h: 117
if _libs["radarkit"].has("RKMomentScratchAlloc", "cdecl"):
    RKMomentScratchAlloc = _libs["radarkit"].get("RKMomentScratchAlloc", "cdecl")
    RKMomentScratchAlloc.argtypes = [POINTER(POINTER(RKMomentScratch)), uint32_t, uint8_t, String]
    RKMomentScratchAlloc.restype = c_size_t

# headers/RadarKit/RKScratch.h: 118
if _libs["radarkit"].has("RKMomentScratchFree", "cdecl"):
    RKMomentScratchFree = _libs["radarkit"].get("RKMomentScratchFree", "cdecl")
    RKMomentScratchFree.argtypes = [POINTER(RKMomentScratch)]
    RKMomentScratchFree.restype = None

# headers/RadarKit/RKScratch.h: 120
if _libs["radarkit"].has("prepareScratch", "cdecl"):
    prepareScratch = _libs["radarkit"].get("prepareScratch", "cdecl")
    prepareScratch.argtypes = [POINTER(RKMomentScratch)]
    prepareScratch.restype = c_int

# headers/RadarKit/RKScratch.h: 121
if _libs["radarkit"].has("makeRayFromScratch", "cdecl"):
    makeRayFromScratch = _libs["radarkit"].get("makeRayFromScratch", "cdecl")
    makeRayFromScratch.argtypes = [POINTER(RKMomentScratch), POINTER(RKRay)]
    makeRayFromScratch.restype = c_int

# headers/RadarKit/RKScratch.h: 123
if _libs["radarkit"].has("RKNullProcessor", "cdecl"):
    RKNullProcessor = _libs["radarkit"].get("RKNullProcessor", "cdecl")
    RKNullProcessor.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKNullProcessor.restype = c_int

# RKPulseEngine.h: 22
class struct_rk_pulse_worker(Structure):
    pass

RKPulseWorker = struct_rk_pulse_worker# RKPulseEngine.h: 17

# RKPulseEngine.h: 40
class struct_rk_pulse_engine(Structure):
    pass

RKPulseEngine = struct_rk_pulse_engine# RKPulseEngine.h: 18

RKPulseEnginePlanIndex = c_int * int(8)# RKPulseEngine.h: 20

struct_rk_pulse_worker.__slots__ = [
    'name',
    'id',
    'tid',
    'parent',
    'semaphoreName',
    'tic',
    'cid',
    'pid',
    'dutyBuff',
    'dutyCycle',
    'lag',
    'sem',
    'compressor',
]
struct_rk_pulse_worker._fields_ = [
    ('name', RKChildName),
    ('id', c_int),
    ('tid', pthread_t),
    ('parent', POINTER(RKPulseEngine)),
    ('semaphoreName', c_char * int(32)),
    ('tic', uint64_t),
    ('cid', uint64_t),
    ('pid', uint32_t),
    ('dutyBuff', c_double * int(1000)),
    ('dutyCycle', c_double),
    ('lag', c_float),
    ('sem', POINTER(sem_t)),
    ('compressor', RKCompressor),
]

struct_rk_pulse_engine.__slots__ = [
    'name',
    'radarDescription',
    'configBuffer',
    'configIndex',
    'pulseBuffer',
    'pulseIndex',
    'doneIndex',
    'oldIndex',
    'fftModule',
    'userModule',
    'verbose',
    'coreCount',
    'coreOrigin',
    'useOldCodes',
    'useSemaphore',
    'filterGroupCount',
    'filterCounts',
    'filterAnchors',
    'filters',
    'compressor',
    'filterGid',
    'planIndices',
    'workers',
    'tidPulseWatcher',
    'mutex',
    'doneStatus',
    'statusBuffer',
    'pulseStatusBuffer',
    'statusBufferIndex',
    'pulseStatusBufferIndex',
    'state',
    'tic',
    'lag',
    'rate',
    'minWorkerLag',
    'maxWorkerLag',
    'almostFull',
    'memoryUsage',
]
struct_rk_pulse_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('doneIndex', uint32_t),
    ('oldIndex', uint32_t),
    ('fftModule', POINTER(RKFFTModule)),
    ('userModule', RKUserModule),
    ('verbose', uint8_t),
    ('coreCount', uint8_t),
    ('coreOrigin', uint8_t),
    ('useOldCodes', c_bool),
    ('useSemaphore', c_bool),
    ('filterGroupCount', uint32_t),
    ('filterCounts', uint32_t * int(22)),
    ('filterAnchors', (RKFilterAnchor * int(8)) * int(22)),
    ('filters', (POINTER(RKComplex) * int(8)) * int(22)),
    ('compressor', CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKCompressionScratch))),
    ('filterGid', POINTER(c_int)),
    ('planIndices', POINTER(RKPulseEnginePlanIndex)),
    ('workers', POINTER(RKPulseWorker)),
    ('tidPulseWatcher', pthread_t),
    ('mutex', pthread_mutex_t),
    ('doneStatus', RKPulseStatus),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('pulseStatusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('pulseStatusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('rate', c_float),
    ('minWorkerLag', c_float),
    ('maxWorkerLag', c_float),
    ('almostFull', c_int),
    ('memoryUsage', c_size_t),
]

# RKPulseEngine.h: 87
if _libs["radarkit"].has("RKPulseEngineInit", "cdecl"):
    RKPulseEngineInit = _libs["radarkit"].get("RKPulseEngineInit", "cdecl")
    RKPulseEngineInit.argtypes = []
    RKPulseEngineInit.restype = POINTER(RKPulseEngine)

# RKPulseEngine.h: 88
if _libs["radarkit"].has("RKPulseEngineFree", "cdecl"):
    RKPulseEngineFree = _libs["radarkit"].get("RKPulseEngineFree", "cdecl")
    RKPulseEngineFree.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineFree.restype = None

# RKPulseEngine.h: 90
if _libs["radarkit"].has("RKPulseEngineSetVerbose", "cdecl"):
    RKPulseEngineSetVerbose = _libs["radarkit"].get("RKPulseEngineSetVerbose", "cdecl")
    RKPulseEngineSetVerbose.argtypes = [POINTER(RKPulseEngine), c_int]
    RKPulseEngineSetVerbose.restype = None

# RKPulseEngine.h: 91
if _libs["radarkit"].has("RKPulseEngineSetEssentials", "cdecl"):
    RKPulseEngineSetEssentials = _libs["radarkit"].get("RKPulseEngineSetEssentials", "cdecl")
    RKPulseEngineSetEssentials.argtypes = [POINTER(RKPulseEngine), POINTER(RKRadarDesc), POINTER(RKFFTModule), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseEngineSetEssentials.restype = None

# RKPulseEngine.h: 94
if _libs["radarkit"].has("RKPulseEngineSetInputOutputBuffers", "cdecl"):
    RKPulseEngineSetInputOutputBuffers = _libs["radarkit"].get("RKPulseEngineSetInputOutputBuffers", "cdecl")
    RKPulseEngineSetInputOutputBuffers.argtypes = [POINTER(RKPulseEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseEngineSetInputOutputBuffers.restype = None

# RKPulseEngine.h: 98
if _libs["radarkit"].has("RKPulseEngineSetFFTModule", "cdecl"):
    RKPulseEngineSetFFTModule = _libs["radarkit"].get("RKPulseEngineSetFFTModule", "cdecl")
    RKPulseEngineSetFFTModule.argtypes = [POINTER(RKPulseEngine), POINTER(RKFFTModule)]
    RKPulseEngineSetFFTModule.restype = None

# RKPulseEngine.h: 99
if _libs["radarkit"].has("RKPulseEngineSetCompressor", "cdecl"):
    RKPulseEngineSetCompressor = _libs["radarkit"].get("RKPulseEngineSetCompressor", "cdecl")
    RKPulseEngineSetCompressor.argtypes = [POINTER(RKPulseEngine), CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKCompressionScratch)), RKUserModule]
    RKPulseEngineSetCompressor.restype = None

# RKPulseEngine.h: 100
if _libs["radarkit"].has("RKPulseEngineUnsetCompressor", "cdecl"):
    RKPulseEngineUnsetCompressor = _libs["radarkit"].get("RKPulseEngineUnsetCompressor", "cdecl")
    RKPulseEngineUnsetCompressor.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineUnsetCompressor.restype = None

# RKPulseEngine.h: 101
if _libs["radarkit"].has("RKPulseEngineSetCoreCount", "cdecl"):
    RKPulseEngineSetCoreCount = _libs["radarkit"].get("RKPulseEngineSetCoreCount", "cdecl")
    RKPulseEngineSetCoreCount.argtypes = [POINTER(RKPulseEngine), uint8_t]
    RKPulseEngineSetCoreCount.restype = None

# RKPulseEngine.h: 102
if _libs["radarkit"].has("RKPulseEngineSetCoreOrigin", "cdecl"):
    RKPulseEngineSetCoreOrigin = _libs["radarkit"].get("RKPulseEngineSetCoreOrigin", "cdecl")
    RKPulseEngineSetCoreOrigin.argtypes = [POINTER(RKPulseEngine), uint8_t]
    RKPulseEngineSetCoreOrigin.restype = None

# RKPulseEngine.h: 103
if _libs["radarkit"].has("RKPulseEngineSetDoneStatus", "cdecl"):
    RKPulseEngineSetDoneStatus = _libs["radarkit"].get("RKPulseEngineSetDoneStatus", "cdecl")
    RKPulseEngineSetDoneStatus.argtypes = [POINTER(RKPulseEngine), RKPulseStatus]
    RKPulseEngineSetDoneStatus.restype = None

# RKPulseEngine.h: 104
if _libs["radarkit"].has("RKPulseEngineSetWaitForRingFilter", "cdecl"):
    RKPulseEngineSetWaitForRingFilter = _libs["radarkit"].get("RKPulseEngineSetWaitForRingFilter", "cdecl")
    RKPulseEngineSetWaitForRingFilter.argtypes = [POINTER(RKPulseEngine), c_bool]
    RKPulseEngineSetWaitForRingFilter.restype = None

# RKPulseEngine.h: 106
if _libs["radarkit"].has("RKPulseEngineResetFilters", "cdecl"):
    RKPulseEngineResetFilters = _libs["radarkit"].get("RKPulseEngineResetFilters", "cdecl")
    RKPulseEngineResetFilters.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineResetFilters.restype = c_int

# RKPulseEngine.h: 107
if _libs["radarkit"].has("RKPulseEngineSetFilterCountOfGroup", "cdecl"):
    RKPulseEngineSetFilterCountOfGroup = _libs["radarkit"].get("RKPulseEngineSetFilterCountOfGroup", "cdecl")
    RKPulseEngineSetFilterCountOfGroup.argtypes = [POINTER(RKPulseEngine), c_int, c_int]
    RKPulseEngineSetFilterCountOfGroup.restype = c_int

# RKPulseEngine.h: 108
if _libs["radarkit"].has("RKPulseEngineSetFilterGroupCount", "cdecl"):
    RKPulseEngineSetFilterGroupCount = _libs["radarkit"].get("RKPulseEngineSetFilterGroupCount", "cdecl")
    RKPulseEngineSetFilterGroupCount.argtypes = [POINTER(RKPulseEngine), c_int]
    RKPulseEngineSetFilterGroupCount.restype = c_int

# RKPulseEngine.h: 109
if _libs["radarkit"].has("RKPulseEngineSetGroupFilter", "cdecl"):
    RKPulseEngineSetGroupFilter = _libs["radarkit"].get("RKPulseEngineSetGroupFilter", "cdecl")
    RKPulseEngineSetGroupFilter.argtypes = [POINTER(RKPulseEngine), POINTER(RKComplex), RKFilterAnchor, c_int, c_int]
    RKPulseEngineSetGroupFilter.restype = c_int

# RKPulseEngine.h: 114
if _libs["radarkit"].has("RKPulseEngineSetFilter", "cdecl"):
    RKPulseEngineSetFilter = _libs["radarkit"].get("RKPulseEngineSetFilter", "cdecl")
    RKPulseEngineSetFilter.argtypes = [POINTER(RKPulseEngine), POINTER(RKComplex), RKFilterAnchor]
    RKPulseEngineSetFilter.restype = c_int

# RKPulseEngine.h: 115
if _libs["radarkit"].has("RKPulseEngineSetFilterByWaveform", "cdecl"):
    RKPulseEngineSetFilterByWaveform = _libs["radarkit"].get("RKPulseEngineSetFilterByWaveform", "cdecl")
    RKPulseEngineSetFilterByWaveform.argtypes = [POINTER(RKPulseEngine), POINTER(RKWaveform)]
    RKPulseEngineSetFilterByWaveform.restype = c_int

# RKPulseEngine.h: 116
if _libs["radarkit"].has("RKPulseEngineSetFilterToImpulse", "cdecl"):
    RKPulseEngineSetFilterToImpulse = _libs["radarkit"].get("RKPulseEngineSetFilterToImpulse", "cdecl")
    RKPulseEngineSetFilterToImpulse.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineSetFilterToImpulse.restype = c_int

# RKPulseEngine.h: 118
if _libs["radarkit"].has("RKPulseEngineStart", "cdecl"):
    RKPulseEngineStart = _libs["radarkit"].get("RKPulseEngineStart", "cdecl")
    RKPulseEngineStart.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineStart.restype = c_int

# RKPulseEngine.h: 119
if _libs["radarkit"].has("RKPulseEngineStop", "cdecl"):
    RKPulseEngineStop = _libs["radarkit"].get("RKPulseEngineStop", "cdecl")
    RKPulseEngineStop.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineStop.restype = c_int

# RKPulseEngine.h: 121
if _libs["radarkit"].has("RKPulseEngineGetVacantPulse", "cdecl"):
    RKPulseEngineGetVacantPulse = _libs["radarkit"].get("RKPulseEngineGetVacantPulse", "cdecl")
    RKPulseEngineGetVacantPulse.argtypes = [POINTER(RKPulseEngine), RKPulseStatus]
    RKPulseEngineGetVacantPulse.restype = POINTER(RKPulse)

# RKPulseEngine.h: 122
if _libs["radarkit"].has("RKPulseEngineGetProcessedPulse", "cdecl"):
    RKPulseEngineGetProcessedPulse = _libs["radarkit"].get("RKPulseEngineGetProcessedPulse", "cdecl")
    RKPulseEngineGetProcessedPulse.argtypes = [POINTER(RKPulseEngine), c_bool]
    RKPulseEngineGetProcessedPulse.restype = POINTER(RKPulse)

# RKPulseEngine.h: 123
if _libs["radarkit"].has("RKPulseEngineWaitWhileBusy", "cdecl"):
    RKPulseEngineWaitWhileBusy = _libs["radarkit"].get("RKPulseEngineWaitWhileBusy", "cdecl")
    RKPulseEngineWaitWhileBusy.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineWaitWhileBusy.restype = None

# RKPulseEngine.h: 125
if _libs["radarkit"].has("RKPulseEngineStatusString", "cdecl"):
    RKPulseEngineStatusString = _libs["radarkit"].get("RKPulseEngineStatusString", "cdecl")
    RKPulseEngineStatusString.argtypes = [POINTER(RKPulseEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseEngineStatusString.restype = ReturnString
    else:
        RKPulseEngineStatusString.restype = String
        RKPulseEngineStatusString.errcheck = ReturnString

# RKPulseEngine.h: 126
if _libs["radarkit"].has("RKPulseEnginePulseString", "cdecl"):
    RKPulseEnginePulseString = _libs["radarkit"].get("RKPulseEnginePulseString", "cdecl")
    RKPulseEnginePulseString.argtypes = [POINTER(RKPulseEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseEnginePulseString.restype = ReturnString
    else:
        RKPulseEnginePulseString.restype = String
        RKPulseEnginePulseString.errcheck = ReturnString

# RKPulseEngine.h: 127
if _libs["radarkit"].has("RKPulseEngineFilterSummary", "cdecl"):
    RKPulseEngineFilterSummary = _libs["radarkit"].get("RKPulseEngineFilterSummary", "cdecl")
    RKPulseEngineFilterSummary.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineFilterSummary.restype = None

# RKPulseEngine.h: 129
if _libs["radarkit"].has("RKBuiltInCompressor", "cdecl"):
    RKBuiltInCompressor = _libs["radarkit"].get("RKBuiltInCompressor", "cdecl")
    RKBuiltInCompressor.argtypes = [RKUserModule, POINTER(RKCompressionScratch)]
    RKBuiltInCompressor.restype = None

# headers/RadarKit/RKWaveform.h: 18
if _libs["radarkit"].has("RKWaveformInitWithCountAndDepth", "cdecl"):
    RKWaveformInitWithCountAndDepth = _libs["radarkit"].get("RKWaveformInitWithCountAndDepth", "cdecl")
    RKWaveformInitWithCountAndDepth.argtypes = [uint8_t, uint32_t]
    RKWaveformInitWithCountAndDepth.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 19
if _libs["radarkit"].has("RKWaveformInitFromSamples", "cdecl"):
    RKWaveformInitFromSamples = _libs["radarkit"].get("RKWaveformInitFromSamples", "cdecl")
    RKWaveformInitFromSamples.argtypes = [POINTER(RKComplex), uint32_t, String]
    RKWaveformInitFromSamples.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 20
if _libs["radarkit"].has("RKWaveformInitFromSampleArrays", "cdecl"):
    RKWaveformInitFromSampleArrays = _libs["radarkit"].get("RKWaveformInitFromSampleArrays", "cdecl")
    RKWaveformInitFromSampleArrays.argtypes = [POINTER(POINTER(RKComplex)), uint8_t, uint32_t, String]
    RKWaveformInitFromSampleArrays.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 21
if _libs["radarkit"].has("RKWaveformInitFromFile", "cdecl"):
    RKWaveformInitFromFile = _libs["radarkit"].get("RKWaveformInitFromFile", "cdecl")
    RKWaveformInitFromFile.argtypes = [String]
    RKWaveformInitFromFile.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 22
if _libs["radarkit"].has("RKWaveformInit", "cdecl"):
    RKWaveformInit = _libs["radarkit"].get("RKWaveformInit", "cdecl")
    RKWaveformInit.argtypes = []
    RKWaveformInit.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 23
if _libs["radarkit"].has("RKWaveformFree", "cdecl"):
    RKWaveformFree = _libs["radarkit"].get("RKWaveformFree", "cdecl")
    RKWaveformFree.argtypes = [POINTER(RKWaveform)]
    RKWaveformFree.restype = None

# headers/RadarKit/RKWaveform.h: 25
if _libs["radarkit"].has("RKWaveformCopy", "cdecl"):
    RKWaveformCopy = _libs["radarkit"].get("RKWaveformCopy", "cdecl")
    RKWaveformCopy.argtypes = [POINTER(RKWaveform)]
    RKWaveformCopy.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 27
if _libs["radarkit"].has("RKWaveformInitAsImpulse", "cdecl"):
    RKWaveformInitAsImpulse = _libs["radarkit"].get("RKWaveformInitAsImpulse", "cdecl")
    RKWaveformInitAsImpulse.argtypes = []
    RKWaveformInitAsImpulse.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 28
if _libs["radarkit"].has("RKWaveformInitAsSingleTone", "cdecl"):
    RKWaveformInitAsSingleTone = _libs["radarkit"].get("RKWaveformInitAsSingleTone", "cdecl")
    RKWaveformInitAsSingleTone.argtypes = [c_double, c_double, c_double]
    RKWaveformInitAsSingleTone.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 29
if _libs["radarkit"].has("RKWaveformInitAsLinearFrequencyModulation", "cdecl"):
    RKWaveformInitAsLinearFrequencyModulation = _libs["radarkit"].get("RKWaveformInitAsLinearFrequencyModulation", "cdecl")
    RKWaveformInitAsLinearFrequencyModulation.argtypes = [c_double, c_double, c_double, c_double]
    RKWaveformInitAsLinearFrequencyModulation.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 30
if _libs["radarkit"].has("RKWaveformInitAsFrequencyHops", "cdecl"):
    RKWaveformInitAsFrequencyHops = _libs["radarkit"].get("RKWaveformInitAsFrequencyHops", "cdecl")
    RKWaveformInitAsFrequencyHops.argtypes = [c_double, c_double, c_double, c_double, c_int]
    RKWaveformInitAsFrequencyHops.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 31
if _libs["radarkit"].has("RKWaveformInitAsFakeTimeFrequencyMultiplexing", "cdecl"):
    RKWaveformInitAsFakeTimeFrequencyMultiplexing = _libs["radarkit"].get("RKWaveformInitAsFakeTimeFrequencyMultiplexing", "cdecl")
    RKWaveformInitAsFakeTimeFrequencyMultiplexing.argtypes = []
    RKWaveformInitAsFakeTimeFrequencyMultiplexing.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 32
if _libs["radarkit"].has("RKWaveformInitAsTimeFrequencyMultiplexing", "cdecl"):
    RKWaveformInitAsTimeFrequencyMultiplexing = _libs["radarkit"].get("RKWaveformInitAsTimeFrequencyMultiplexing", "cdecl")
    RKWaveformInitAsTimeFrequencyMultiplexing.argtypes = [c_double, c_double, c_double, c_double]
    RKWaveformInitAsTimeFrequencyMultiplexing.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 33
if _libs["radarkit"].has("RKWaveformInitAsFrequencyHoppingChirp", "cdecl"):
    RKWaveformInitAsFrequencyHoppingChirp = _libs["radarkit"].get("RKWaveformInitAsFrequencyHoppingChirp", "cdecl")
    RKWaveformInitAsFrequencyHoppingChirp.argtypes = [c_double, c_double, c_double, c_double, c_int]
    RKWaveformInitAsFrequencyHoppingChirp.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 34
if _libs["radarkit"].has("RKWaveformInitFromString", "cdecl"):
    RKWaveformInitFromString = _libs["radarkit"].get("RKWaveformInitFromString", "cdecl")
    RKWaveformInitFromString.argtypes = [String]
    RKWaveformInitFromString.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 36
if _libs["radarkit"].has("RKWaveformAppendWaveform", "cdecl"):
    RKWaveformAppendWaveform = _libs["radarkit"].get("RKWaveformAppendWaveform", "cdecl")
    RKWaveformAppendWaveform.argtypes = [POINTER(RKWaveform), POINTER(RKWaveform), uint32_t]
    RKWaveformAppendWaveform.restype = RKResult

# headers/RadarKit/RKWaveform.h: 37
if _libs["radarkit"].has("RKWaveformApplyWindow", "cdecl"):
    _func = _libs["radarkit"].get("RKWaveformApplyWindow", "cdecl")
    _restype = RKResult
    _errcheck = None
    _argtypes = [POINTER(RKWaveform), RKWindowType]
    RKWaveformApplyWindow = _variadic_function(_func,_restype,_argtypes,_errcheck)

# headers/RadarKit/RKWaveform.h: 39
if _libs["radarkit"].has("RKWaveformOnes", "cdecl"):
    RKWaveformOnes = _libs["radarkit"].get("RKWaveformOnes", "cdecl")
    RKWaveformOnes.argtypes = [POINTER(RKWaveform)]
    RKWaveformOnes.restype = None

# headers/RadarKit/RKWaveform.h: 40
if _libs["radarkit"].has("RKWaveformSingleTone", "cdecl"):
    RKWaveformSingleTone = _libs["radarkit"].get("RKWaveformSingleTone", "cdecl")
    RKWaveformSingleTone.argtypes = [POINTER(RKWaveform), c_double, c_double]
    RKWaveformSingleTone.restype = None

# headers/RadarKit/RKWaveform.h: 41
if _libs["radarkit"].has("RKWaveformLinearFrequencyModulation", "cdecl"):
    RKWaveformLinearFrequencyModulation = _libs["radarkit"].get("RKWaveformLinearFrequencyModulation", "cdecl")
    RKWaveformLinearFrequencyModulation.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformLinearFrequencyModulation.restype = None

# headers/RadarKit/RKWaveform.h: 42
if _libs["radarkit"].has("RKWaveformFrequencyHops", "cdecl"):
    RKWaveformFrequencyHops = _libs["radarkit"].get("RKWaveformFrequencyHops", "cdecl")
    RKWaveformFrequencyHops.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformFrequencyHops.restype = None

# headers/RadarKit/RKWaveform.h: 43
if _libs["radarkit"].has("RKWaveformFrequencyHoppingChirp", "cdecl"):
    RKWaveformFrequencyHoppingChirp = _libs["radarkit"].get("RKWaveformFrequencyHoppingChirp", "cdecl")
    RKWaveformFrequencyHoppingChirp.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformFrequencyHoppingChirp.restype = None

# headers/RadarKit/RKWaveform.h: 45
if _libs["radarkit"].has("RKWaveformDecimate", "cdecl"):
    RKWaveformDecimate = _libs["radarkit"].get("RKWaveformDecimate", "cdecl")
    RKWaveformDecimate.argtypes = [POINTER(RKWaveform), c_int]
    RKWaveformDecimate.restype = None

# headers/RadarKit/RKWaveform.h: 46
if _libs["radarkit"].has("RKWaveformConjugate", "cdecl"):
    RKWaveformConjugate = _libs["radarkit"].get("RKWaveformConjugate", "cdecl")
    RKWaveformConjugate.argtypes = [POINTER(RKWaveform)]
    RKWaveformConjugate.restype = None

# headers/RadarKit/RKWaveform.h: 47
if _libs["radarkit"].has("RKWaveformDownConvert", "cdecl"):
    RKWaveformDownConvert = _libs["radarkit"].get("RKWaveformDownConvert", "cdecl")
    RKWaveformDownConvert.argtypes = [POINTER(RKWaveform)]
    RKWaveformDownConvert.restype = None

# headers/RadarKit/RKWaveform.h: 49
if _libs["radarkit"].has("RKWaveformNormalizeNoiseGain", "cdecl"):
    RKWaveformNormalizeNoiseGain = _libs["radarkit"].get("RKWaveformNormalizeNoiseGain", "cdecl")
    RKWaveformNormalizeNoiseGain.argtypes = [POINTER(RKWaveform)]
    RKWaveformNormalizeNoiseGain.restype = None

# headers/RadarKit/RKWaveform.h: 50
if _libs["radarkit"].has("RKWaveformSummary", "cdecl"):
    RKWaveformSummary = _libs["radarkit"].get("RKWaveformSummary", "cdecl")
    RKWaveformSummary.argtypes = [POINTER(RKWaveform)]
    RKWaveformSummary.restype = None

# headers/RadarKit/RKWaveform.h: 52
if _libs["radarkit"].has("RKWaveformWriteToReference", "cdecl"):
    RKWaveformWriteToReference = _libs["radarkit"].get("RKWaveformWriteToReference", "cdecl")
    RKWaveformWriteToReference.argtypes = [POINTER(RKWaveform), POINTER(FILE)]
    RKWaveformWriteToReference.restype = c_size_t

# headers/RadarKit/RKWaveform.h: 53
if _libs["radarkit"].has("RKWaveformWriteFile", "cdecl"):
    RKWaveformWriteFile = _libs["radarkit"].get("RKWaveformWriteFile", "cdecl")
    RKWaveformWriteFile.argtypes = [POINTER(RKWaveform), String]
    RKWaveformWriteFile.restype = RKResult

# headers/RadarKit/RKWaveform.h: 54
if _libs["radarkit"].has("RKWaveformReadFromReference", "cdecl"):
    RKWaveformReadFromReference = _libs["radarkit"].get("RKWaveformReadFromReference", "cdecl")
    RKWaveformReadFromReference.argtypes = [POINTER(FILE)]
    RKWaveformReadFromReference.restype = POINTER(RKWaveform)

# RKFileHeader.h: 15
if _libs["radarkit"].has("RKFileHeaderInit", "cdecl"):
    RKFileHeaderInit = _libs["radarkit"].get("RKFileHeaderInit", "cdecl")
    RKFileHeaderInit.argtypes = []
    RKFileHeaderInit.restype = POINTER(RKFileHeader)

# RKFileHeader.h: 16
if _libs["radarkit"].has("RKFileHeaderFree", "cdecl"):
    RKFileHeaderFree = _libs["radarkit"].get("RKFileHeaderFree", "cdecl")
    RKFileHeaderFree.argtypes = [POINTER(RKFileHeader)]
    RKFileHeaderFree.restype = None

# RKFileHeader.h: 18
if _libs["radarkit"].has("RKFileHeaderInitFromFid", "cdecl"):
    RKFileHeaderInitFromFid = _libs["radarkit"].get("RKFileHeaderInitFromFid", "cdecl")
    RKFileHeaderInitFromFid.argtypes = [POINTER(FILE)]
    RKFileHeaderInitFromFid.restype = POINTER(RKFileHeader)

# RKFileHeader.h: 19
if _libs["radarkit"].has("RKFileHeaderWriteToFid", "cdecl"):
    RKFileHeaderWriteToFid = _libs["radarkit"].get("RKFileHeaderWriteToFid", "cdecl")
    RKFileHeaderWriteToFid.argtypes = [POINTER(RKFileHeader), POINTER(FILE)]
    RKFileHeaderWriteToFid.restype = c_size_t

# RKFileHeader.h: 21
if _libs["radarkit"].has("RKFileHeaderSummary", "cdecl"):
    RKFileHeaderSummary = _libs["radarkit"].get("RKFileHeaderSummary", "cdecl")
    RKFileHeaderSummary.argtypes = [POINTER(RKFileHeader)]
    RKFileHeaderSummary.restype = None

# headers/RadarKit/RKFileManager.h: 65
class struct_rk_file_remover(Structure):
    pass

RKFileRemover = struct_rk_file_remover# headers/RadarKit/RKFileManager.h: 62

# headers/RadarKit/RKFileManager.h: 87
class struct_rk_file_manager(Structure):
    pass

RKFileManager = struct_rk_file_manager# headers/RadarKit/RKFileManager.h: 63

struct_rk_file_remover.__slots__ = [
    'name',
    'id',
    'tic',
    'tid',
    'parent',
    'index',
    'count',
    'capacity',
    'usage',
    'limit',
    'path',
    'folders',
    'filenames',
    'indexedStats',
    'reusable',
    'latestTime',
]
struct_rk_file_remover._fields_ = [
    ('name', RKChildName),
    ('id', c_int),
    ('tic', uint64_t),
    ('tid', pthread_t),
    ('parent', POINTER(RKFileManager)),
    ('index', c_int),
    ('count', c_int),
    ('capacity', c_int),
    ('usage', c_size_t),
    ('limit', c_size_t),
    ('path', c_char * int((768 + 384))),
    ('folders', POINTER(None)),
    ('filenames', POINTER(None)),
    ('indexedStats', POINTER(None)),
    ('reusable', c_bool),
    ('latestTime', struct_timeval),
]

struct_rk_file_manager.__slots__ = [
    'name',
    'radarDescription',
    'verbose',
    'dataPath',
    'usagelimit',
    'maximumLogAgeInDays',
    'userRawDataUsageLimit',
    'userMomentDataUsageLimit',
    'userHealthDataUsageLimit',
    'tic',
    'workerCount',
    'workers',
    'tidFileWatcher',
    'mutex',
    'scratch',
    'state',
    'memoryUsage',
]
struct_rk_file_manager._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('verbose', uint8_t),
    ('dataPath', c_char * int(768)),
    ('usagelimit', c_size_t),
    ('maximumLogAgeInDays', c_int),
    ('userRawDataUsageLimit', c_size_t),
    ('userMomentDataUsageLimit', c_size_t),
    ('userHealthDataUsageLimit', c_size_t),
    ('tic', uint64_t),
    ('workerCount', c_int),
    ('workers', POINTER(RKFileRemover)),
    ('tidFileWatcher', pthread_t),
    ('mutex', pthread_mutex_t),
    ('scratch', c_char * int(4096)),
    ('state', RKEngineState),
    ('memoryUsage', uint32_t),
]

# RKRawDataRecorder.h: 21
class struct_rk_data_recorder(Structure):
    pass

RKRawDataRecorder = struct_rk_data_recorder# RKRawDataRecorder.h: 19

struct_rk_data_recorder.__slots__ = [
    'name',
    'radarDescription',
    'rawDataType',
    'pulseBuffer',
    'pulseIndex',
    'configBuffer',
    'configIndex',
    'verbose',
    'record',
    'cacheSize',
    'maximumRecordDepth',
    'fileManager',
    'filename',
    'fd',
    'fid',
    'cache',
    'cacheWriteIndex',
    'cacheFlushCount',
    'fileWriteCount',
    'fileWriteSize',
    'filePulseCount',
    'tidPulseRecorder',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'lag',
    'memoryUsage',
]
struct_rk_data_recorder._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('rawDataType', RKRawDataType),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('record', c_bool),
    ('cacheSize', c_size_t),
    ('maximumRecordDepth', c_size_t),
    ('fileManager', POINTER(RKFileManager)),
    ('filename', c_char * int(1024)),
    ('fd', c_int),
    ('fid', POINTER(FILE)),
    ('cache', POINTER(None)),
    ('cacheWriteIndex', c_size_t),
    ('cacheFlushCount', uint64_t),
    ('fileWriteCount', uint64_t),
    ('fileWriteSize', uint64_t),
    ('filePulseCount', uint64_t),
    ('tidPulseRecorder', pthread_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('memoryUsage', c_size_t),
]

# RKRawDataRecorder.h: 57
if _libs["radarkit"].has("RKRawDataRecorderInit", "cdecl"):
    RKRawDataRecorderInit = _libs["radarkit"].get("RKRawDataRecorderInit", "cdecl")
    RKRawDataRecorderInit.argtypes = []
    RKRawDataRecorderInit.restype = POINTER(RKRawDataRecorder)

# RKRawDataRecorder.h: 58
if _libs["radarkit"].has("RKRawDataRecorderFree", "cdecl"):
    RKRawDataRecorderFree = _libs["radarkit"].get("RKRawDataRecorderFree", "cdecl")
    RKRawDataRecorderFree.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderFree.restype = None

# RKRawDataRecorder.h: 60
if _libs["radarkit"].has("RKRawDataRecorderSetVerbose", "cdecl"):
    RKRawDataRecorderSetVerbose = _libs["radarkit"].get("RKRawDataRecorderSetVerbose", "cdecl")
    RKRawDataRecorderSetVerbose.argtypes = [POINTER(RKRawDataRecorder), c_int]
    RKRawDataRecorderSetVerbose.restype = None

# RKRawDataRecorder.h: 61
if _libs["radarkit"].has("RKRawDataRecorderSetEssentials", "cdecl"):
    RKRawDataRecorderSetEssentials = _libs["radarkit"].get("RKRawDataRecorderSetEssentials", "cdecl")
    RKRawDataRecorderSetEssentials.argtypes = [POINTER(RKRawDataRecorder), POINTER(RKRadarDesc), POINTER(RKFileManager), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKRawDataRecorderSetEssentials.restype = None

# RKRawDataRecorder.h: 64
if _libs["radarkit"].has("RKRawDataRecorderSetRecord", "cdecl"):
    RKRawDataRecorderSetRecord = _libs["radarkit"].get("RKRawDataRecorderSetRecord", "cdecl")
    RKRawDataRecorderSetRecord.argtypes = [POINTER(RKRawDataRecorder), c_bool]
    RKRawDataRecorderSetRecord.restype = None

# RKRawDataRecorder.h: 65
if _libs["radarkit"].has("RKRawDataRecorderSetRawDataType", "cdecl"):
    RKRawDataRecorderSetRawDataType = _libs["radarkit"].get("RKRawDataRecorderSetRawDataType", "cdecl")
    RKRawDataRecorderSetRawDataType.argtypes = [POINTER(RKRawDataRecorder), RKRawDataType]
    RKRawDataRecorderSetRawDataType.restype = None

# RKRawDataRecorder.h: 66
if _libs["radarkit"].has("RKRawDataRecorderSetMaximumRecordDepth", "cdecl"):
    RKRawDataRecorderSetMaximumRecordDepth = _libs["radarkit"].get("RKRawDataRecorderSetMaximumRecordDepth", "cdecl")
    RKRawDataRecorderSetMaximumRecordDepth.argtypes = [POINTER(RKRawDataRecorder), uint32_t]
    RKRawDataRecorderSetMaximumRecordDepth.restype = None

# RKRawDataRecorder.h: 67
if _libs["radarkit"].has("RKRawDataRecorderSetCacheSize", "cdecl"):
    RKRawDataRecorderSetCacheSize = _libs["radarkit"].get("RKRawDataRecorderSetCacheSize", "cdecl")
    RKRawDataRecorderSetCacheSize.argtypes = [POINTER(RKRawDataRecorder), uint32_t]
    RKRawDataRecorderSetCacheSize.restype = None

# RKRawDataRecorder.h: 69
if _libs["radarkit"].has("RKRawDataRecorderStart", "cdecl"):
    RKRawDataRecorderStart = _libs["radarkit"].get("RKRawDataRecorderStart", "cdecl")
    RKRawDataRecorderStart.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderStart.restype = c_int

# RKRawDataRecorder.h: 70
if _libs["radarkit"].has("RKRawDataRecorderStop", "cdecl"):
    RKRawDataRecorderStop = _libs["radarkit"].get("RKRawDataRecorderStop", "cdecl")
    RKRawDataRecorderStop.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderStop.restype = c_int

# RKRawDataRecorder.h: 71
if _libs["radarkit"].has("RKRawDataRecorderStatusString", "cdecl"):
    RKRawDataRecorderStatusString = _libs["radarkit"].get("RKRawDataRecorderStatusString", "cdecl")
    RKRawDataRecorderStatusString.argtypes = [POINTER(RKRawDataRecorder)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKRawDataRecorderStatusString.restype = ReturnString
    else:
        RKRawDataRecorderStatusString.restype = String
        RKRawDataRecorderStatusString.errcheck = ReturnString

# RKRawDataRecorder.h: 73
if _libs["radarkit"].has("RKRawDataRecorderNewFile", "cdecl"):
    RKRawDataRecorderNewFile = _libs["radarkit"].get("RKRawDataRecorderNewFile", "cdecl")
    RKRawDataRecorderNewFile.argtypes = [POINTER(RKRawDataRecorder), String]
    RKRawDataRecorderNewFile.restype = c_int

# RKRawDataRecorder.h: 74
if _libs["radarkit"].has("RKRawDataRecorderCloseFile", "cdecl"):
    RKRawDataRecorderCloseFile = _libs["radarkit"].get("RKRawDataRecorderCloseFile", "cdecl")
    RKRawDataRecorderCloseFile.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderCloseFile.restype = c_int

# RKRawDataRecorder.h: 75
if _libs["radarkit"].has("RKRawDataRecorderCloseFileQuiet", "cdecl"):
    RKRawDataRecorderCloseFileQuiet = _libs["radarkit"].get("RKRawDataRecorderCloseFileQuiet", "cdecl")
    RKRawDataRecorderCloseFileQuiet.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderCloseFileQuiet.restype = c_int

# RKRawDataRecorder.h: 76
if _libs["radarkit"].has("RKRawDataRecorderIncreasePulseCount", "cdecl"):
    RKRawDataRecorderIncreasePulseCount = _libs["radarkit"].get("RKRawDataRecorderIncreasePulseCount", "cdecl")
    RKRawDataRecorderIncreasePulseCount.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderIncreasePulseCount.restype = c_int

# RKRawDataRecorder.h: 77
if _libs["radarkit"].has("RKRawDataRecorderCacheWrite", "cdecl"):
    RKRawDataRecorderCacheWrite = _libs["radarkit"].get("RKRawDataRecorderCacheWrite", "cdecl")
    RKRawDataRecorderCacheWrite.argtypes = [POINTER(RKRawDataRecorder), POINTER(None), c_size_t]
    RKRawDataRecorderCacheWrite.restype = c_size_t

# RKRawDataRecorder.h: 78
if _libs["radarkit"].has("RKRawDataRecorderCacheFlush", "cdecl"):
    RKRawDataRecorderCacheFlush = _libs["radarkit"].get("RKRawDataRecorderCacheFlush", "cdecl")
    RKRawDataRecorderCacheFlush.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderCacheFlush.restype = c_size_t

# headers/RadarKit/RKPulsePair.h: 15
if _libs["radarkit"].has("RKPulsePair", "cdecl"):
    RKPulsePair = _libs["radarkit"].get("RKPulsePair", "cdecl")
    RKPulsePair.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulsePair.restype = c_int

# headers/RadarKit/RKPulsePair.h: 16
if _libs["radarkit"].has("RKPulsePairHop", "cdecl"):
    RKPulsePairHop = _libs["radarkit"].get("RKPulsePairHop", "cdecl")
    RKPulsePairHop.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulsePairHop.restype = c_int

# headers/RadarKit/RKPulsePair.h: 17
if _libs["radarkit"].has("RKPulsePairStaggeredPRT", "cdecl"):
    RKPulsePairStaggeredPRT = _libs["radarkit"].get("RKPulsePairStaggeredPRT", "cdecl")
    RKPulsePairStaggeredPRT.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulsePairStaggeredPRT.restype = c_int

# headers/RadarKit/RKMultiLag.h: 15
if _libs["radarkit"].has("RKMultiLag", "cdecl"):
    RKMultiLag = _libs["radarkit"].get("RKMultiLag", "cdecl")
    RKMultiLag.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKMultiLag.restype = c_int

# headers/RadarKit/RKNoiseEstimator.h: 15
if _libs["radarkit"].has("RKRayNoiseEstimator", "cdecl"):
    RKRayNoiseEstimator = _libs["radarkit"].get("RKRayNoiseEstimator", "cdecl")
    RKRayNoiseEstimator.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKRayNoiseEstimator.restype = c_int

# headers/RadarKit/RKSpectralMoment.h: 15
if _libs["radarkit"].has("RKSpectralMoment", "cdecl"):
    RKSpectralMoment = _libs["radarkit"].get("RKSpectralMoment", "cdecl")
    RKSpectralMoment.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKSpectralMoment.restype = c_int

# headers/RadarKit/RKPulsePairATSR.h: 15
if _libs["radarkit"].has("RKPulsePairATSR", "cdecl"):
    RKPulsePairATSR = _libs["radarkit"].get("RKPulsePairATSR", "cdecl")
    RKPulsePairATSR.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulsePairATSR.restype = c_int

# RKMomentEngine.h: 26
class struct_rk_moment_worker(Structure):
    pass

RKMomentWorker = struct_rk_moment_worker# RKMomentEngine.h: 23

# RKMomentEngine.h: 41
class struct_rk_moment_engine(Structure):
    pass

RKMomentEngine = struct_rk_moment_engine# RKMomentEngine.h: 24

struct_rk_moment_worker.__slots__ = [
    'name',
    'id',
    'tid',
    'parent',
    'semaphoreName',
    'tic',
    'pid',
    'dutyBuff',
    'dutyCycle',
    'lag',
    'sem',
]
struct_rk_moment_worker._fields_ = [
    ('name', RKChildName),
    ('id', c_int),
    ('tid', pthread_t),
    ('parent', POINTER(RKMomentEngine)),
    ('semaphoreName', c_char * int(32)),
    ('tic', uint64_t),
    ('pid', uint32_t),
    ('dutyBuff', c_double * int(1000)),
    ('dutyCycle', c_double),
    ('lag', c_float),
    ('sem', POINTER(sem_t)),
]

struct_rk_moment_engine.__slots__ = [
    'name',
    'radarDescription',
    'configBuffer',
    'configIndex',
    'pulseBuffer',
    'pulseIndex',
    'rayBuffer',
    'rayIndex',
    'doneIndex',
    'oldIndex',
    'fftModule',
    'userModule',
    'verbose',
    'coreCount',
    'coreOrigin',
    'useOldCodes',
    'useSemaphore',
    'excludeBoundaryPulses',
    'noiseEstimator',
    'momentProcessor',
    'calibrator',
    'momentSource',
    'workers',
    'tidPulseGatherer',
    'mutex',
    'processorLagCount',
    'processorFFTOrder',
    'userLagChoice',
    'business',
    'processedPulseIndex',
    'statusBuffer',
    'rayStatusBuffer',
    'statusBufferIndex',
    'rayStatusBufferIndex',
    'state',
    'tic',
    'lag',
    'minWorkerLag',
    'maxWorkerLag',
    'almostFull',
    'memoryUsage',
]
struct_rk_moment_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('rayBuffer', RKBuffer),
    ('rayIndex', POINTER(uint32_t)),
    ('doneIndex', uint32_t),
    ('oldIndex', uint32_t),
    ('fftModule', POINTER(RKFFTModule)),
    ('userModule', RKUserModule),
    ('verbose', uint8_t),
    ('coreCount', uint8_t),
    ('coreOrigin', uint8_t),
    ('useOldCodes', c_bool),
    ('useSemaphore', c_bool),
    ('excludeBoundaryPulses', c_bool),
    ('noiseEstimator', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)),
    ('momentProcessor', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)),
    ('calibrator', CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKMomentScratch))),
    ('momentSource', POINTER(RKModuloPath)),
    ('workers', POINTER(RKMomentWorker)),
    ('tidPulseGatherer', pthread_t),
    ('mutex', pthread_mutex_t),
    ('processorLagCount', uint8_t),
    ('processorFFTOrder', uint8_t),
    ('userLagChoice', uint8_t),
    ('business', uint32_t),
    ('processedPulseIndex', uint32_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('rayStatusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('rayStatusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('minWorkerLag', c_float),
    ('maxWorkerLag', c_float),
    ('almostFull', uint32_t),
    ('memoryUsage', c_size_t),
]

# RKMomentEngine.h: 90
if _libs["radarkit"].has("RKMomentEngineInit", "cdecl"):
    RKMomentEngineInit = _libs["radarkit"].get("RKMomentEngineInit", "cdecl")
    RKMomentEngineInit.argtypes = []
    RKMomentEngineInit.restype = POINTER(RKMomentEngine)

# RKMomentEngine.h: 91
if _libs["radarkit"].has("RKMomentEngineFree", "cdecl"):
    RKMomentEngineFree = _libs["radarkit"].get("RKMomentEngineFree", "cdecl")
    RKMomentEngineFree.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineFree.restype = None

# RKMomentEngine.h: 93
if _libs["radarkit"].has("RKMomentEngineSetVerbose", "cdecl"):
    RKMomentEngineSetVerbose = _libs["radarkit"].get("RKMomentEngineSetVerbose", "cdecl")
    RKMomentEngineSetVerbose.argtypes = [POINTER(RKMomentEngine), c_int]
    RKMomentEngineSetVerbose.restype = None

# RKMomentEngine.h: 94
if _libs["radarkit"].has("RKMomentEngineSetEssentials", "cdecl"):
    RKMomentEngineSetEssentials = _libs["radarkit"].get("RKMomentEngineSetEssentials", "cdecl")
    RKMomentEngineSetEssentials.argtypes = [POINTER(RKMomentEngine), POINTER(RKRadarDesc), POINTER(RKFFTModule), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKMomentEngineSetEssentials.restype = None

# RKMomentEngine.h: 98
if _libs["radarkit"].has("RKMomentEngineSetInputOutputBuffers", "cdecl"):
    RKMomentEngineSetInputOutputBuffers = _libs["radarkit"].get("RKMomentEngineSetInputOutputBuffers", "cdecl")
    RKMomentEngineSetInputOutputBuffers.argtypes = [POINTER(RKMomentEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKMomentEngineSetInputOutputBuffers.restype = None

# RKMomentEngine.h: 103
if _libs["radarkit"].has("RKMomentEngineSetFFTModule", "cdecl"):
    RKMomentEngineSetFFTModule = _libs["radarkit"].get("RKMomentEngineSetFFTModule", "cdecl")
    RKMomentEngineSetFFTModule.argtypes = [POINTER(RKMomentEngine), POINTER(RKFFTModule)]
    RKMomentEngineSetFFTModule.restype = None

# RKMomentEngine.h: 104
if _libs["radarkit"].has("RKMomentEngineSetCalibrator", "cdecl"):
    RKMomentEngineSetCalibrator = _libs["radarkit"].get("RKMomentEngineSetCalibrator", "cdecl")
    RKMomentEngineSetCalibrator.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKMomentScratch)), RKUserModule]
    RKMomentEngineSetCalibrator.restype = None

# RKMomentEngine.h: 105
if _libs["radarkit"].has("RKMomentEngineUnsetCalibrator", "cdecl"):
    RKMomentEngineUnsetCalibrator = _libs["radarkit"].get("RKMomentEngineUnsetCalibrator", "cdecl")
    RKMomentEngineUnsetCalibrator.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineUnsetCalibrator.restype = None

# RKMomentEngine.h: 106
if _libs["radarkit"].has("RKMomentEngineSetCoreCount", "cdecl"):
    RKMomentEngineSetCoreCount = _libs["radarkit"].get("RKMomentEngineSetCoreCount", "cdecl")
    RKMomentEngineSetCoreCount.argtypes = [POINTER(RKMomentEngine), uint8_t]
    RKMomentEngineSetCoreCount.restype = None

# RKMomentEngine.h: 107
if _libs["radarkit"].has("RKMomentEngineSetCoreOrigin", "cdecl"):
    RKMomentEngineSetCoreOrigin = _libs["radarkit"].get("RKMomentEngineSetCoreOrigin", "cdecl")
    RKMomentEngineSetCoreOrigin.argtypes = [POINTER(RKMomentEngine), uint8_t]
    RKMomentEngineSetCoreOrigin.restype = None

# RKMomentEngine.h: 108
if _libs["radarkit"].has("RKMomentEngineSetExcludeBoundaryPulses", "cdecl"):
    RKMomentEngineSetExcludeBoundaryPulses = _libs["radarkit"].get("RKMomentEngineSetExcludeBoundaryPulses", "cdecl")
    RKMomentEngineSetExcludeBoundaryPulses.argtypes = [POINTER(RKMomentEngine), c_bool]
    RKMomentEngineSetExcludeBoundaryPulses.restype = None

# RKMomentEngine.h: 109
if _libs["radarkit"].has("RKMomentEngineSetNoiseEstimator", "cdecl"):
    RKMomentEngineSetNoiseEstimator = _libs["radarkit"].get("RKMomentEngineSetNoiseEstimator", "cdecl")
    RKMomentEngineSetNoiseEstimator.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)]
    RKMomentEngineSetNoiseEstimator.restype = None

# RKMomentEngine.h: 110
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessor", "cdecl"):
    RKMomentEngineSetMomentProcessor = _libs["radarkit"].get("RKMomentEngineSetMomentProcessor", "cdecl")
    RKMomentEngineSetMomentProcessor.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)]
    RKMomentEngineSetMomentProcessor.restype = None

# RKMomentEngine.h: 111
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToPulsePair", "cdecl"):
    RKMomentEngineSetMomentProcessorToPulsePair = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToPulsePair", "cdecl")
    RKMomentEngineSetMomentProcessorToPulsePair.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToPulsePair.restype = None

# RKMomentEngine.h: 112
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToPulsePairHop", "cdecl"):
    RKMomentEngineSetMomentProcessorToPulsePairHop = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToPulsePairHop", "cdecl")
    RKMomentEngineSetMomentProcessorToPulsePairHop.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToPulsePairHop.restype = None

# RKMomentEngine.h: 113
if _libs["radarkit"].has("RKMomentEngineSetMomentPRocessorToPulsePairATSR", "cdecl"):
    RKMomentEngineSetMomentPRocessorToPulsePairATSR = _libs["radarkit"].get("RKMomentEngineSetMomentPRocessorToPulsePairATSR", "cdecl")
    RKMomentEngineSetMomentPRocessorToPulsePairATSR.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentPRocessorToPulsePairATSR.restype = None

# RKMomentEngine.h: 114
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToMultiLag2", "cdecl"):
    RKMomentEngineSetMomentProcessorToMultiLag2 = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToMultiLag2", "cdecl")
    RKMomentEngineSetMomentProcessorToMultiLag2.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToMultiLag2.restype = None

# RKMomentEngine.h: 115
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToMultiLag3", "cdecl"):
    RKMomentEngineSetMomentProcessorToMultiLag3 = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToMultiLag3", "cdecl")
    RKMomentEngineSetMomentProcessorToMultiLag3.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToMultiLag3.restype = None

# RKMomentEngine.h: 116
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToMultiLag4", "cdecl"):
    RKMomentEngineSetMomentProcessorToMultiLag4 = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToMultiLag4", "cdecl")
    RKMomentEngineSetMomentProcessorToMultiLag4.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToMultiLag4.restype = None

# RKMomentEngine.h: 117
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessorToSpectral", "cdecl"):
    RKMomentEngineSetMomentProcessorToSpectral = _libs["radarkit"].get("RKMomentEngineSetMomentProcessorToSpectral", "cdecl")
    RKMomentEngineSetMomentProcessorToSpectral.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineSetMomentProcessorToSpectral.restype = None

# RKMomentEngine.h: 119
if _libs["radarkit"].has("RKMomentEngineStart", "cdecl"):
    RKMomentEngineStart = _libs["radarkit"].get("RKMomentEngineStart", "cdecl")
    RKMomentEngineStart.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineStart.restype = c_int

# RKMomentEngine.h: 120
if _libs["radarkit"].has("RKMomentEngineStop", "cdecl"):
    RKMomentEngineStop = _libs["radarkit"].get("RKMomentEngineStop", "cdecl")
    RKMomentEngineStop.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineStop.restype = c_int

# RKMomentEngine.h: 122
if _libs["radarkit"].has("RKMomentEngineGetProcessedRay", "cdecl"):
    RKMomentEngineGetProcessedRay = _libs["radarkit"].get("RKMomentEngineGetProcessedRay", "cdecl")
    RKMomentEngineGetProcessedRay.argtypes = [POINTER(RKMomentEngine), c_bool]
    RKMomentEngineGetProcessedRay.restype = POINTER(RKRay)

# RKMomentEngine.h: 123
if _libs["radarkit"].has("RKMomentEngineFlush", "cdecl"):
    RKMomentEngineFlush = _libs["radarkit"].get("RKMomentEngineFlush", "cdecl")
    RKMomentEngineFlush.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineFlush.restype = None

# RKMomentEngine.h: 124
if _libs["radarkit"].has("RKMomentEngineWaitWhileBusy", "cdecl"):
    RKMomentEngineWaitWhileBusy = _libs["radarkit"].get("RKMomentEngineWaitWhileBusy", "cdecl")
    RKMomentEngineWaitWhileBusy.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineWaitWhileBusy.restype = None

# RKMomentEngine.h: 126
if _libs["radarkit"].has("RKMomentEngineStatusString", "cdecl"):
    RKMomentEngineStatusString = _libs["radarkit"].get("RKMomentEngineStatusString", "cdecl")
    RKMomentEngineStatusString.argtypes = [POINTER(RKMomentEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKMomentEngineStatusString.restype = ReturnString
    else:
        RKMomentEngineStatusString.restype = String
        RKMomentEngineStatusString.errcheck = ReturnString

# RKMomentEngine.h: 128
if _libs["radarkit"].has("RKNoiseFromConfig", "cdecl"):
    RKNoiseFromConfig = _libs["radarkit"].get("RKNoiseFromConfig", "cdecl")
    RKNoiseFromConfig.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKNoiseFromConfig.restype = c_int

# headers/RadarKit/RKProduct.h: 14
if _libs["radarkit"].has("RKProductBufferAlloc", "cdecl"):
    RKProductBufferAlloc = _libs["radarkit"].get("RKProductBufferAlloc", "cdecl")
    RKProductBufferAlloc.argtypes = [POINTER(POINTER(RKProduct)), uint32_t, uint32_t, uint32_t]
    RKProductBufferAlloc.restype = c_size_t

# headers/RadarKit/RKProduct.h: 15
if _libs["radarkit"].has("RKProductBufferExtend", "cdecl"):
    RKProductBufferExtend = _libs["radarkit"].get("RKProductBufferExtend", "cdecl")
    RKProductBufferExtend.argtypes = [POINTER(POINTER(RKProduct)), uint32_t, uint32_t]
    RKProductBufferExtend.restype = c_size_t

# headers/RadarKit/RKProduct.h: 16
if _libs["radarkit"].has("RKProductBufferFree", "cdecl"):
    RKProductBufferFree = _libs["radarkit"].get("RKProductBufferFree", "cdecl")
    RKProductBufferFree.argtypes = [POINTER(RKProduct), uint32_t]
    RKProductBufferFree.restype = None

# headers/RadarKit/RKProduct.h: 18
if _libs["radarkit"].has("RKProductInitFromSweep", "cdecl"):
    RKProductInitFromSweep = _libs["radarkit"].get("RKProductInitFromSweep", "cdecl")
    RKProductInitFromSweep.argtypes = [POINTER(RKProduct), POINTER(RKSweep)]
    RKProductInitFromSweep.restype = c_int

# headers/RadarKit/RKProduct.h: 19
if _libs["radarkit"].has("RKProductFree", "cdecl"):
    RKProductFree = _libs["radarkit"].get("RKProductFree", "cdecl")
    RKProductFree.argtypes = [POINTER(RKProduct)]
    RKProductFree.restype = None

# headers/RadarKit/RKProductFile.h: 31
if _libs["radarkit"].has("RKProductDimensionsFromFile", "cdecl"):
    RKProductDimensionsFromFile = _libs["radarkit"].get("RKProductDimensionsFromFile", "cdecl")
    RKProductDimensionsFromFile.argtypes = [String, POINTER(uint32_t), POINTER(uint32_t)]
    RKProductDimensionsFromFile.restype = None

# headers/RadarKit/RKProductFile.h: 33
if _libs["radarkit"].has("RKProductFileWriterNC", "cdecl"):
    RKProductFileWriterNC = _libs["radarkit"].get("RKProductFileWriterNC", "cdecl")
    RKProductFileWriterNC.argtypes = [POINTER(RKProduct), String]
    RKProductFileWriterNC.restype = c_int

# headers/RadarKit/RKProductFile.h: 37
if _libs["radarkit"].has("RKProductCollectionInit", "cdecl"):
    RKProductCollectionInit = _libs["radarkit"].get("RKProductCollectionInit", "cdecl")
    RKProductCollectionInit.argtypes = [c_int, uint32_t, uint32_t]
    RKProductCollectionInit.restype = POINTER(RKProductCollection)

# headers/RadarKit/RKProductFile.h: 38
if _libs["radarkit"].has("RKProductCollectionInitFromSingles", "cdecl"):
    RKProductCollectionInitFromSingles = _libs["radarkit"].get("RKProductCollectionInitFromSingles", "cdecl")
    RKProductCollectionInitFromSingles.argtypes = [POINTER(POINTER(RKProductCollection)), uint32_t]
    RKProductCollectionInitFromSingles.restype = POINTER(RKProductCollection)

# headers/RadarKit/RKProductFile.h: 39
if _libs["radarkit"].has("RKProductCollectionFree", "cdecl"):
    RKProductCollectionFree = _libs["radarkit"].get("RKProductCollectionFree", "cdecl")
    RKProductCollectionFree.argtypes = [POINTER(RKProductCollection)]
    RKProductCollectionFree.restype = None

# headers/RadarKit/RKProductFile.h: 41
if _libs["radarkit"].has("RKProductCollectionInitWithFilename", "cdecl"):
    RKProductCollectionInitWithFilename = _libs["radarkit"].get("RKProductCollectionInitWithFilename", "cdecl")
    RKProductCollectionInitWithFilename.argtypes = [String]
    RKProductCollectionInitWithFilename.restype = POINTER(RKProductCollection)

# headers/RadarKit/RKProductFile.h: 44
if _libs["radarkit"].has("RKProductCollectionStandardizeForCFRadial", "cdecl"):
    RKProductCollectionStandardizeForCFRadial = _libs["radarkit"].get("RKProductCollectionStandardizeForCFRadial", "cdecl")
    RKProductCollectionStandardizeForCFRadial.argtypes = [POINTER(RKProductCollection)]
    RKProductCollectionStandardizeForCFRadial.restype = c_int

# headers/RadarKit/RKProductFile.h: 45
if _libs["radarkit"].has("RKProductCollectionFileWriterCF", "cdecl"):
    RKProductCollectionFileWriterCF = _libs["radarkit"].get("RKProductCollectionFileWriterCF", "cdecl")
    RKProductCollectionFileWriterCF.argtypes = [POINTER(RKProductCollection), String, RKWriterOption]
    RKProductCollectionFileWriterCF.restype = c_int

# RKSweepEngine.h: 27
class struct_rk_sweep_scratch(Structure):
    pass

struct_rk_sweep_scratch.__slots__ = [
    'filelist',
    'summary',
    'rays',
    'rayCount',
]
struct_rk_sweep_scratch._fields_ = [
    ('filelist', c_char * int((1024 + (20 * 1024)))),
    ('summary', c_char * int(4096)),
    ('rays', POINTER(RKRay) * int(1500)),
    ('rayCount', uint32_t),
]

RKSweepScratchSpace = struct_rk_sweep_scratch# RKSweepEngine.h: 27

# RKSweepEngine.h: 31
class struct_rk_sweep_engine(Structure):
    pass

RKSweepEngine = struct_rk_sweep_engine# RKSweepEngine.h: 29

struct_rk_sweep_engine.__slots__ = [
    'name',
    'radarDescription',
    'rayBuffer',
    'rayIndex',
    'configBuffer',
    'configIndex',
    'verbose',
    'record',
    'hasFileHandlingScript',
    'fileHandlingScript',
    'fileHandlingScriptProperties',
    'fileManager',
    'productFileExtension',
    'productRecorder',
    'productCollectionRecorder',
    'sweepIndex',
    'tidRayGatherer',
    'scratchSpaces',
    'scratchSpaceIndex',
    'lastRecordedScratchSpaceIndex',
    'productBuffer',
    'productBufferDepth',
    'productIndex',
    'productMutex',
    'productList',
    'productIds',
    'business',
    'processedRayIndex',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'lag',
    'almostFull',
    'memoryUsage',
]
struct_rk_sweep_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('rayBuffer', RKBuffer),
    ('rayIndex', POINTER(uint32_t)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('record', c_bool),
    ('hasFileHandlingScript', c_bool),
    ('fileHandlingScript', c_char * int(1024)),
    ('fileHandlingScriptProperties', RKScriptProperty),
    ('fileManager', POINTER(RKFileManager)),
    ('productFileExtension', c_char * int(8)),
    ('productRecorder', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKProduct), String)),
    ('productCollectionRecorder', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKProductCollection), String, uint32_t)),
    ('sweepIndex', RKIdentifier),
    ('tidRayGatherer', pthread_t),
    ('scratchSpaces', RKSweepScratchSpace * int(6)),
    ('scratchSpaceIndex', uint8_t),
    ('lastRecordedScratchSpaceIndex', uint8_t),
    ('productBuffer', POINTER(RKProduct)),
    ('productBufferDepth', uint32_t),
    ('productIndex', uint32_t),
    ('productMutex', pthread_mutex_t),
    ('productList', RKProductList),
    ('productIds', RKProductId * int(RKProductIndexCount)),
    ('business', uint32_t),
    ('processedRayIndex', uint32_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('almostFull', uint32_t),
    ('memoryUsage', c_size_t),
]

# RKSweepEngine.h: 74
if _libs["radarkit"].has("RKSweepEngineInit", "cdecl"):
    RKSweepEngineInit = _libs["radarkit"].get("RKSweepEngineInit", "cdecl")
    RKSweepEngineInit.argtypes = []
    RKSweepEngineInit.restype = POINTER(RKSweepEngine)

# RKSweepEngine.h: 75
if _libs["radarkit"].has("RKSweepEngineFree", "cdecl"):
    RKSweepEngineFree = _libs["radarkit"].get("RKSweepEngineFree", "cdecl")
    RKSweepEngineFree.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineFree.restype = None

# RKSweepEngine.h: 77
if _libs["radarkit"].has("RKSweepEngineSetVerbose", "cdecl"):
    RKSweepEngineSetVerbose = _libs["radarkit"].get("RKSweepEngineSetVerbose", "cdecl")
    RKSweepEngineSetVerbose.argtypes = [POINTER(RKSweepEngine), c_int]
    RKSweepEngineSetVerbose.restype = None

# RKSweepEngine.h: 78
if _libs["radarkit"].has("RKSweepEngineSetEssentials", "cdecl"):
    RKSweepEngineSetEssentials = _libs["radarkit"].get("RKSweepEngineSetEssentials", "cdecl")
    RKSweepEngineSetEssentials.argtypes = [POINTER(RKSweepEngine), POINTER(RKRadarDesc), POINTER(RKFileManager), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKSweepEngineSetEssentials.restype = None

# RKSweepEngine.h: 81
if _libs["radarkit"].has("RKSweepEngineSetRecord", "cdecl"):
    RKSweepEngineSetRecord = _libs["radarkit"].get("RKSweepEngineSetRecord", "cdecl")
    RKSweepEngineSetRecord.argtypes = [POINTER(RKSweepEngine), c_bool]
    RKSweepEngineSetRecord.restype = None

# RKSweepEngine.h: 82
for _lib in _libs.values():
    if not _lib.has("RKSweepEngineSetProductTimeout", "cdecl"):
        continue
    RKSweepEngineSetProductTimeout = _lib.get("RKSweepEngineSetProductTimeout", "cdecl")
    RKSweepEngineSetProductTimeout.argtypes = [POINTER(RKSweepEngine), uint32_t]
    RKSweepEngineSetProductTimeout.restype = None
    break

# RKSweepEngine.h: 83
if _libs["radarkit"].has("RKSweepEngineSetFilesHandlingScript", "cdecl"):
    RKSweepEngineSetFilesHandlingScript = _libs["radarkit"].get("RKSweepEngineSetFilesHandlingScript", "cdecl")
    RKSweepEngineSetFilesHandlingScript.argtypes = [POINTER(RKSweepEngine), String, RKScriptProperty]
    RKSweepEngineSetFilesHandlingScript.restype = None

# RKSweepEngine.h: 84
if _libs["radarkit"].has("RKSweepEngineSetProductRecorder", "cdecl"):
    RKSweepEngineSetProductRecorder = _libs["radarkit"].get("RKSweepEngineSetProductRecorder", "cdecl")
    RKSweepEngineSetProductRecorder.argtypes = [POINTER(RKSweepEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKProduct), String)]
    RKSweepEngineSetProductRecorder.restype = None

# RKSweepEngine.h: 86
if _libs["radarkit"].has("RKSweepEngineStart", "cdecl"):
    RKSweepEngineStart = _libs["radarkit"].get("RKSweepEngineStart", "cdecl")
    RKSweepEngineStart.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineStart.restype = c_int

# RKSweepEngine.h: 87
if _libs["radarkit"].has("RKSweepEngineStop", "cdecl"):
    RKSweepEngineStop = _libs["radarkit"].get("RKSweepEngineStop", "cdecl")
    RKSweepEngineStop.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineStop.restype = c_int

# RKSweepEngine.h: 88
if _libs["radarkit"].has("RKSweepEngineFlush", "cdecl"):
    RKSweepEngineFlush = _libs["radarkit"].get("RKSweepEngineFlush", "cdecl")
    RKSweepEngineFlush.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineFlush.restype = None

# RKSweepEngine.h: 90
if _libs["radarkit"].has("RKSweepEngineStatusString", "cdecl"):
    RKSweepEngineStatusString = _libs["radarkit"].get("RKSweepEngineStatusString", "cdecl")
    RKSweepEngineStatusString.argtypes = [POINTER(RKSweepEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSweepEngineStatusString.restype = ReturnString
    else:
        RKSweepEngineStatusString.restype = String
        RKSweepEngineStatusString.errcheck = ReturnString

# RKSweepEngine.h: 91
if _libs["radarkit"].has("RKSweepEngineLatestSummary", "cdecl"):
    RKSweepEngineLatestSummary = _libs["radarkit"].get("RKSweepEngineLatestSummary", "cdecl")
    RKSweepEngineLatestSummary.argtypes = [POINTER(RKSweepEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSweepEngineLatestSummary.restype = ReturnString
    else:
        RKSweepEngineLatestSummary.restype = String
        RKSweepEngineLatestSummary.errcheck = ReturnString

# RKSweepEngine.h: 93
if _libs["radarkit"].has("RKSweepEngineDescribeProduct", "cdecl"):
    RKSweepEngineDescribeProduct = _libs["radarkit"].get("RKSweepEngineDescribeProduct", "cdecl")
    RKSweepEngineDescribeProduct.argtypes = [POINTER(RKSweepEngine), RKProductDesc]
    RKSweepEngineDescribeProduct.restype = RKProductId

# RKSweepEngine.h: 94
if _libs["radarkit"].has("RKSweepEngineUndescribeProduct", "cdecl"):
    RKSweepEngineUndescribeProduct = _libs["radarkit"].get("RKSweepEngineUndescribeProduct", "cdecl")
    RKSweepEngineUndescribeProduct.argtypes = [POINTER(RKSweepEngine), RKProductId]
    RKSweepEngineUndescribeProduct.restype = c_int

# RKSweepEngine.h: 95
if _libs["radarkit"].has("RKSweepEngineGetVacantProduct", "cdecl"):
    RKSweepEngineGetVacantProduct = _libs["radarkit"].get("RKSweepEngineGetVacantProduct", "cdecl")
    RKSweepEngineGetVacantProduct.argtypes = [POINTER(RKSweepEngine), POINTER(RKSweep), RKProductId]
    RKSweepEngineGetVacantProduct.restype = POINTER(RKProduct)

# RKSweepEngine.h: 96
if _libs["radarkit"].has("RKSweepEngineSetProductComplete", "cdecl"):
    RKSweepEngineSetProductComplete = _libs["radarkit"].get("RKSweepEngineSetProductComplete", "cdecl")
    RKSweepEngineSetProductComplete.argtypes = [POINTER(RKSweepEngine), POINTER(RKSweep), POINTER(RKProduct)]
    RKSweepEngineSetProductComplete.restype = c_int

# RKSweepEngine.h: 97
if _libs["radarkit"].has("RKSweepEngineWaitWhileBusy", "cdecl"):
    RKSweepEngineWaitWhileBusy = _libs["radarkit"].get("RKSweepEngineWaitWhileBusy", "cdecl")
    RKSweepEngineWaitWhileBusy.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineWaitWhileBusy.restype = None

# RKSweepEngine.h: 99
if _libs["radarkit"].has("RKSweepCollect", "cdecl"):
    RKSweepCollect = _libs["radarkit"].get("RKSweepCollect", "cdecl")
    RKSweepCollect.argtypes = [POINTER(RKSweepEngine), uint8_t]
    RKSweepCollect.restype = POINTER(RKSweep)

# RKSweepEngine.h: 100
if _libs["radarkit"].has("RKSweepFree", "cdecl"):
    RKSweepFree = _libs["radarkit"].get("RKSweepFree", "cdecl")
    RKSweepFree.argtypes = [POINTER(RKSweep)]
    RKSweepFree.restype = c_int

# RKPulseRingFilter.h: 21
class struct_rk_pulse_ring_filter_worker(Structure):
    pass

RKPulseRingFilterWorker = struct_rk_pulse_ring_filter_worker# RKPulseRingFilter.h: 18

# RKPulseRingFilter.h: 39
class struct_rk_pulse_ring_filter_engine(Structure):
    pass

RKPulseRingFilterEngine = struct_rk_pulse_ring_filter_engine# RKPulseRingFilter.h: 19

struct_rk_pulse_ring_filter_worker.__slots__ = [
    'name',
    'id',
    'tid',
    'parent',
    'semaphoreName',
    'tic',
    'pid',
    'processOrigin',
    'processLength',
    'outputLength',
    'dutyBuff',
    'dutyCycle',
    'lag',
    'sem',
]
struct_rk_pulse_ring_filter_worker._fields_ = [
    ('name', RKChildName),
    ('id', c_int),
    ('tid', pthread_t),
    ('parent', POINTER(RKPulseRingFilterEngine)),
    ('semaphoreName', c_char * int(32)),
    ('tic', uint64_t),
    ('pid', uint32_t),
    ('processOrigin', uint32_t),
    ('processLength', uint32_t),
    ('outputLength', uint32_t),
    ('dutyBuff', c_double * int(1000)),
    ('dutyCycle', c_double),
    ('lag', c_float),
    ('sem', POINTER(sem_t)),
]

struct_rk_pulse_ring_filter_engine.__slots__ = [
    'name',
    'radarDescription',
    'pulseBuffer',
    'pulseIndex',
    'configBuffer',
    'configIndex',
    'verbose',
    'coreCount',
    'coreOrigin',
    'useOldCodes',
    'useSemaphore',
    'useFilter',
    'filter',
    'filterId',
    'workers',
    'workerTaskDone',
    'tidPulseWatcher',
    'mutex',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'lag',
    'minWorkerLag',
    'maxWorkerLag',
    'almostFull',
    'memoryUsage',
]
struct_rk_pulse_ring_filter_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('coreCount', uint8_t),
    ('coreOrigin', uint8_t),
    ('useOldCodes', c_bool),
    ('useSemaphore', c_bool),
    ('useFilter', c_bool),
    ('filter', RKIIRFilter),
    ('filterId', RKIdentifier),
    ('workers', POINTER(RKPulseRingFilterWorker)),
    ('workerTaskDone', POINTER(c_bool)),
    ('tidPulseWatcher', pthread_t),
    ('mutex', pthread_mutex_t),
    ('statusBuffer', (c_char * int(4096)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('minWorkerLag', c_float),
    ('maxWorkerLag', c_float),
    ('almostFull', c_int),
    ('memoryUsage', c_size_t),
]

# RKPulseRingFilter.h: 74
if _libs["radarkit"].has("RKPulseRingFilterEngineInit", "cdecl"):
    RKPulseRingFilterEngineInit = _libs["radarkit"].get("RKPulseRingFilterEngineInit", "cdecl")
    RKPulseRingFilterEngineInit.argtypes = []
    RKPulseRingFilterEngineInit.restype = POINTER(RKPulseRingFilterEngine)

# RKPulseRingFilter.h: 75
if _libs["radarkit"].has("RKPulseRingFilterEngineFree", "cdecl"):
    RKPulseRingFilterEngineFree = _libs["radarkit"].get("RKPulseRingFilterEngineFree", "cdecl")
    RKPulseRingFilterEngineFree.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineFree.restype = None

# RKPulseRingFilter.h: 77
if _libs["radarkit"].has("RKPulseRingFilterEngineSetVerbose", "cdecl"):
    RKPulseRingFilterEngineSetVerbose = _libs["radarkit"].get("RKPulseRingFilterEngineSetVerbose", "cdecl")
    RKPulseRingFilterEngineSetVerbose.argtypes = [POINTER(RKPulseRingFilterEngine), c_int]
    RKPulseRingFilterEngineSetVerbose.restype = None

# RKPulseRingFilter.h: 78
if _libs["radarkit"].has("RKPulseRingFilterEngineSetEssentials", "cdecl"):
    RKPulseRingFilterEngineSetEssentials = _libs["radarkit"].get("RKPulseRingFilterEngineSetEssentials", "cdecl")
    RKPulseRingFilterEngineSetEssentials.argtypes = [POINTER(RKPulseRingFilterEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseRingFilterEngineSetEssentials.restype = None

# RKPulseRingFilter.h: 81
if _libs["radarkit"].has("RKPulseRingFilterEngineSetCoreCount", "cdecl"):
    RKPulseRingFilterEngineSetCoreCount = _libs["radarkit"].get("RKPulseRingFilterEngineSetCoreCount", "cdecl")
    RKPulseRingFilterEngineSetCoreCount.argtypes = [POINTER(RKPulseRingFilterEngine), uint8_t]
    RKPulseRingFilterEngineSetCoreCount.restype = None

# RKPulseRingFilter.h: 82
if _libs["radarkit"].has("RKPulseRingFilterEngineSetCoreOrigin", "cdecl"):
    RKPulseRingFilterEngineSetCoreOrigin = _libs["radarkit"].get("RKPulseRingFilterEngineSetCoreOrigin", "cdecl")
    RKPulseRingFilterEngineSetCoreOrigin.argtypes = [POINTER(RKPulseRingFilterEngine), uint8_t]
    RKPulseRingFilterEngineSetCoreOrigin.restype = None

# RKPulseRingFilter.h: 84
if _libs["radarkit"].has("RKPulseRingFilterEngineEnableFilter", "cdecl"):
    RKPulseRingFilterEngineEnableFilter = _libs["radarkit"].get("RKPulseRingFilterEngineEnableFilter", "cdecl")
    RKPulseRingFilterEngineEnableFilter.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineEnableFilter.restype = None

# RKPulseRingFilter.h: 85
if _libs["radarkit"].has("RKPulseRingFilterEngineDisableFilter", "cdecl"):
    RKPulseRingFilterEngineDisableFilter = _libs["radarkit"].get("RKPulseRingFilterEngineDisableFilter", "cdecl")
    RKPulseRingFilterEngineDisableFilter.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineDisableFilter.restype = None

# RKPulseRingFilter.h: 86
if _libs["radarkit"].has("RKPulseRingFilterEngineSetFilter", "cdecl"):
    RKPulseRingFilterEngineSetFilter = _libs["radarkit"].get("RKPulseRingFilterEngineSetFilter", "cdecl")
    RKPulseRingFilterEngineSetFilter.argtypes = [POINTER(RKPulseRingFilterEngine), POINTER(RKIIRFilter)]
    RKPulseRingFilterEngineSetFilter.restype = c_int

# RKPulseRingFilter.h: 88
if _libs["radarkit"].has("RKPulseRingFilterEngineStart", "cdecl"):
    RKPulseRingFilterEngineStart = _libs["radarkit"].get("RKPulseRingFilterEngineStart", "cdecl")
    RKPulseRingFilterEngineStart.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineStart.restype = c_int

# RKPulseRingFilter.h: 89
if _libs["radarkit"].has("RKPulseRingFilterEngineStop", "cdecl"):
    RKPulseRingFilterEngineStop = _libs["radarkit"].get("RKPulseRingFilterEngineStop", "cdecl")
    RKPulseRingFilterEngineStop.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineStop.restype = c_int

# RKPulseRingFilter.h: 91
if _libs["radarkit"].has("RKPulseRingFilterEngineShowFilterSummary", "cdecl"):
    RKPulseRingFilterEngineShowFilterSummary = _libs["radarkit"].get("RKPulseRingFilterEngineShowFilterSummary", "cdecl")
    RKPulseRingFilterEngineShowFilterSummary.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineShowFilterSummary.restype = None

# RKPulseRingFilter.h: 92
if _libs["radarkit"].has("RKPulseRingFilterEngineStatusString", "cdecl"):
    RKPulseRingFilterEngineStatusString = _libs["radarkit"].get("RKPulseRingFilterEngineStatusString", "cdecl")
    RKPulseRingFilterEngineStatusString.argtypes = [POINTER(RKPulseRingFilterEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseRingFilterEngineStatusString.restype = ReturnString
    else:
        RKPulseRingFilterEngineStatusString.restype = String
        RKPulseRingFilterEngineStatusString.errcheck = ReturnString

# RKPulsePairHop.h: 15
if _libs["radarkit"].has("RKPulsePairHop", "cdecl"):
    RKPulsePairHop = _libs["radarkit"].get("RKPulsePairHop", "cdecl")
    RKPulsePairHop.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulsePairHop.restype = c_int

RKNetworkSocketType = c_int# headers/RadarKit/RKNetwork.h: 21

RKNetworkMessageFormat = c_int# headers/RadarKit/RKNetwork.h: 27

# headers/RadarKit/RKNetwork.h: 57
class struct_anon_172(Structure):
    pass

struct_anon_172._pack_ = 1
struct_anon_172.__slots__ = [
    'type',
    'subtype',
    'size',
    'decodedSize',
]
struct_anon_172._fields_ = [
    ('type', uint16_t),
    ('subtype', uint16_t),
    ('size', uint32_t),
    ('decodedSize', uint32_t),
]

# headers/RadarKit/RKNetwork.h: 64
class union_rk_net_delimiter(Union):
    pass

union_rk_net_delimiter._pack_ = 1
union_rk_net_delimiter.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_net_delimiter._anonymous_ = [
    'unnamed_1',
]
union_rk_net_delimiter._fields_ = [
    ('unnamed_1', struct_anon_172),
    ('bytes', RKByte * int(16)),
]

RKNetDelimiter = union_rk_net_delimiter# headers/RadarKit/RKNetwork.h: 64

RKClientState = uint32_t# headers/RadarKit/RKClient.h: 18

# headers/RadarKit/RKClient.h: 48
class struct_rk_client(Structure):
    pass

RKClient = struct_rk_client# headers/RadarKit/RKClient.h: 46

struct_rk_client.__slots__ = [
    'name',
    'hostname',
    'port',
    'timeoutSeconds',
    'type',
    'format',
    'blockLength',
    'blocking',
    'reconnect',
    'ping',
    'verbose',
    'userResource',
    'userPayload',
    'init',
    'recv',
    'exit',
    'hostIP',
    'sa',
    'sd',
    'ireq',
    'state',
    'safeToClose',
    'threadId',
    'threadAttributes',
    'lock',
    'rfd',
    'wfd',
    'efd',
    'netDelimiter',
]
struct_rk_client._fields_ = [
    ('name', RKName),
    ('hostname', RKName),
    ('port', c_int),
    ('timeoutSeconds', c_int),
    ('type', RKNetworkSocketType),
    ('format', RKNetworkMessageFormat),
    ('blockLength', c_int),
    ('blocking', c_bool),
    ('reconnect', c_bool),
    ('ping', c_bool),
    ('verbose', c_int),
    ('userResource', POINTER(None)),
    ('userPayload', POINTER(None)),
    ('init', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKClient))),
    ('recv', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKClient))),
    ('exit', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKClient))),
    ('hostIP', c_char * int(32)),
    ('sa', struct_sockaddr_in),
    ('sd', c_int),
    ('ireq', c_int),
    ('state', RKClientState),
    ('safeToClose', c_bool),
    ('threadId', pthread_t),
    ('threadAttributes', pthread_attr_t),
    ('lock', pthread_mutex_t),
    ('rfd', fd_set),
    ('wfd', fd_set),
    ('efd', fd_set),
    ('netDelimiter', RKNetDelimiter),
]

# headers/RadarKit/RKClock.h: 56
class struct_rk_clock(Structure):
    pass

struct_rk_clock.__slots__ = [
    'name',
    'verbose',
    'autoSync',
    'hasWisdom',
    'infoShown',
    'highPrecision',
    'useInternalReference',
    'offsetSeconds',
    'size',
    'block',
    'stride',
    'tic',
    'tBuffer',
    'xBuffer',
    'uBuffer',
    'yBuffer',
    'zBuffer',
    'a',
    'b',
    'index',
    'count',
    'initDay',
    'initTime',
    'latestTime',
    'typicalPeriod',
    'x0',
    'u0',
    'dx',
    'sum_x0',
    'sum_u0',
]
struct_rk_clock._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('autoSync', c_bool),
    ('hasWisdom', c_bool),
    ('infoShown', c_bool),
    ('highPrecision', c_bool),
    ('useInternalReference', c_bool),
    ('offsetSeconds', c_double),
    ('size', uint32_t),
    ('block', uint32_t),
    ('stride', uint32_t),
    ('tic', uint64_t),
    ('tBuffer', POINTER(struct_timeval)),
    ('xBuffer', POINTER(c_double)),
    ('uBuffer', POINTER(c_double)),
    ('yBuffer', POINTER(c_double)),
    ('zBuffer', POINTER(c_double)),
    ('a', c_double),
    ('b', c_double),
    ('index', uint32_t),
    ('count', uint64_t),
    ('initDay', c_double),
    ('initTime', c_double),
    ('latestTime', c_double),
    ('typicalPeriod', c_double),
    ('x0', c_double),
    ('u0', c_double),
    ('dx', c_double),
    ('sum_x0', c_double),
    ('sum_u0', c_double),
]

RKClock = struct_rk_clock# headers/RadarKit/RKClock.h: 56

# headers/RadarKit/RKHealthEngine.h: 18
class struct_rk_health_engine(Structure):
    pass

RKHealthEngine = struct_rk_health_engine# headers/RadarKit/RKHealthEngine.h: 16

struct_rk_health_engine.__slots__ = [
    'name',
    'radarDescription',
    'healthNodes',
    'healthBuffer',
    'healthIndex',
    'verbose',
    'fid',
    'tidHealthConsolidator',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'memoryUsage',
]
struct_rk_health_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('healthNodes', POINTER(RKNodalHealth)),
    ('healthBuffer', POINTER(RKHealth)),
    ('healthIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('fid', POINTER(FILE)),
    ('tidHealthConsolidator', pthread_t),
    ('statusBuffer', (c_char * int(4096)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('memoryUsage', c_size_t),
]

# headers/RadarKit/RKPositionEngine.h: 53
class struct_rk_position_engine(Structure):
    pass

RKPositionEngine = struct_rk_position_engine# headers/RadarKit/RKPositionEngine.h: 51

struct_rk_position_engine.__slots__ = [
    'name',
    'radarDescription',
    'configBuffer',
    'configIndex',
    'positionBuffer',
    'positionIndex',
    'pulseBuffer',
    'pulseIndex',
    'verbose',
    'pedestal',
    'hardwareInit',
    'hardwareExec',
    'hardwareRead',
    'hardwareFree',
    'hardwareInitInput',
    'threadId',
    'startTime',
    'processedPulseIndex',
    'statusBuffer',
    'positionStringBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'lag',
    'memoryUsage',
    'vcpI',
    'vcpSweepCount',
]
struct_rk_position_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('positionBuffer', POINTER(RKPosition)),
    ('positionIndex', POINTER(uint32_t)),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('pedestal', RKPedestal),
    ('hardwareInit', CFUNCTYPE(UNCHECKED(RKPedestal), POINTER(None))),
    ('hardwareExec', CFUNCTYPE(UNCHECKED(c_int), RKPedestal, String)),
    ('hardwareRead', CFUNCTYPE(UNCHECKED(c_int), RKPedestal, POINTER(RKPosition))),
    ('hardwareFree', CFUNCTYPE(UNCHECKED(c_int), RKPedestal)),
    ('hardwareInitInput', POINTER(None)),
    ('threadId', pthread_t),
    ('startTime', c_double),
    ('processedPulseIndex', uint32_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('positionStringBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('memoryUsage', c_size_t),
    ('vcpI', c_int),
    ('vcpSweepCount', c_int),
]

RKScanProgress = uint8_t# headers/RadarKit/RKSteerEngine.h: 41

RKScanOption = uint8_t# headers/RadarKit/RKSteerEngine.h: 51

RKScanMode = uint8_t# headers/RadarKit/RKSteerEngine.h: 59

# headers/RadarKit/RKSteerEngine.h: 111
class struct_rk_scan_path(Structure):
    pass

struct_rk_scan_path.__slots__ = [
    'mode',
    'azimuthStart',
    'azimuthEnd',
    'azimuthSlew',
    'azimuthMark',
    'elevationStart',
    'elevationEnd',
    'elevationSlew',
]
struct_rk_scan_path._fields_ = [
    ('mode', RKScanMode),
    ('azimuthStart', c_float),
    ('azimuthEnd', c_float),
    ('azimuthSlew', c_float),
    ('azimuthMark', c_float),
    ('elevationStart', c_float),
    ('elevationEnd', c_float),
    ('elevationSlew', c_float),
]

RKScanPath = struct_rk_scan_path# headers/RadarKit/RKSteerEngine.h: 111

# headers/RadarKit/RKSteerEngine.h: 130
class struct_rk_scan_object(Structure):
    pass

struct_rk_scan_object.__slots__ = [
    'name',
    'option',
    'batterScans',
    'onDeckScans',
    'inTheHoleScans',
    'inTheHoleCount',
    'onDeckCount',
    'sweepCount',
    'active',
    'i',
    'tic',
    'toc',
    'elevationPrevious',
    'azimuthPrevious',
    'progress',
    'lastAction',
]
struct_rk_scan_object._fields_ = [
    ('name', RKName),
    ('option', RKScanOption),
    ('batterScans', RKScanPath * int(256)),
    ('onDeckScans', RKScanPath * int(256)),
    ('inTheHoleScans', RKScanPath * int(256)),
    ('inTheHoleCount', uint16_t),
    ('onDeckCount', uint16_t),
    ('sweepCount', uint16_t),
    ('active', c_bool),
    ('i', c_int),
    ('tic', c_int),
    ('toc', c_int),
    ('elevationPrevious', c_float),
    ('azimuthPrevious', c_float),
    ('progress', RKScanProgress),
    ('lastAction', RKScanAction),
]

RKScanObject = struct_rk_scan_object# headers/RadarKit/RKSteerEngine.h: 130

# headers/RadarKit/RKSteerEngine.h: 134
class struct_rk_position_steer_engine(Structure):
    pass

RKSteerEngine = struct_rk_position_steer_engine# headers/RadarKit/RKSteerEngine.h: 132

struct_rk_position_steer_engine.__slots__ = [
    'name',
    'radarDescription',
    'positionBuffer',
    'positionIndex',
    'configBuffer',
    'configIndex',
    'verbose',
    'vcpHandle',
    'actions',
    'actionIndex',
    'volumeIndex',
    'sweepIndex',
    'scanString',
    'response',
    'dump',
    'threadId',
    'memoryUsage',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'lag',
]
struct_rk_position_steer_engine._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('positionBuffer', POINTER(RKPosition)),
    ('positionIndex', POINTER(uint32_t)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('vcpHandle', RKScanObject),
    ('actions', RKScanAction * int(8)),
    ('actionIndex', uint32_t),
    ('volumeIndex', uint32_t),
    ('sweepIndex', uint32_t),
    ('scanString', c_char * int(4096)),
    ('response', c_char * int(4096)),
    ('dump', c_char * int(4096)),
    ('threadId', pthread_t),
    ('memoryUsage', c_size_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
]

# headers/RadarKit/RKHealthLogger.h: 17
class struct_rk_health_logger(Structure):
    pass

RKHealthLogger = struct_rk_health_logger# headers/RadarKit/RKHealthLogger.h: 15

struct_rk_health_logger.__slots__ = [
    'name',
    'radarDescription',
    'healthBuffer',
    'healthIndex',
    'healthBufferDepth',
    'verbose',
    'record',
    'healthRelay',
    'fileManager',
    'fid',
    'tidBackground',
    'statusBuffer',
    'statusBufferIndex',
    'state',
    'tic',
    'memoryUsage',
]
struct_rk_health_logger._fields_ = [
    ('name', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('healthBuffer', POINTER(RKHealth)),
    ('healthIndex', POINTER(uint32_t)),
    ('healthBufferDepth', uint32_t),
    ('verbose', uint8_t),
    ('record', c_bool),
    ('healthRelay', RKHealthRelay),
    ('fileManager', POINTER(RKFileManager)),
    ('fid', POINTER(FILE)),
    ('tidBackground', pthread_t),
    ('statusBuffer', (c_char * int(4096)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('memoryUsage', c_size_t),
]

# headers/RadarKit/RKRadarRelay.h: 64
class struct_rk_radar_relay(Structure):
    pass

struct_rk_radar_relay.__slots__ = [
    'name',
    'host',
    'radarDescription',
    'configBuffer',
    'configIndex',
    'healthBuffer',
    'healthIndex',
    'statusBuffer',
    'statusIndex',
    'pulseBuffer',
    'pulseIndex',
    'rayBuffer',
    'rayIndex',
    'verbose',
    'fileManager',
    'client',
    'responseIndex',
    'responses',
    'latestCommand',
    'tidBackground',
    'streams',
    'sweepHeaderCache',
    'sweepPacketCount',
    'sweepRayIndex',
    'sweepTic',
    'sweepToc',
    'pulseStatusBuffer',
    'rayStatusBuffer',
    'pulseStatusBufferIndex',
    'rayStatusBufferIndex',
    'state',
    'tic',
    'memoryUsage',
]
struct_rk_radar_relay._fields_ = [
    ('name', RKName),
    ('host', RKName),
    ('radarDescription', POINTER(RKRadarDesc)),
    ('configBuffer', POINTER(RKConfig)),
    ('configIndex', POINTER(uint32_t)),
    ('healthBuffer', POINTER(RKHealth)),
    ('healthIndex', POINTER(uint32_t)),
    ('statusBuffer', POINTER(RKStatus)),
    ('statusIndex', POINTER(uint32_t)),
    ('pulseBuffer', RKBuffer),
    ('pulseIndex', POINTER(uint32_t)),
    ('rayBuffer', RKBuffer),
    ('rayIndex', POINTER(uint32_t)),
    ('verbose', uint8_t),
    ('fileManager', POINTER(RKFileManager)),
    ('client', POINTER(RKClient)),
    ('responseIndex', uint32_t),
    ('responses', (c_char * int(8196)) * int(200)),
    ('latestCommand', c_char * int(512)),
    ('tidBackground', pthread_t),
    ('streams', RKStream),
    ('sweepHeaderCache', RKSweepHeader),
    ('sweepPacketCount', uint32_t),
    ('sweepRayIndex', uint32_t),
    ('sweepTic', struct_timeval),
    ('sweepToc', struct_timeval),
    ('pulseStatusBuffer', (c_char * int(4096)) * int(10)),
    ('rayStatusBuffer', (c_char * int(4096)) * int(10)),
    ('pulseStatusBufferIndex', uint32_t),
    ('rayStatusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('memoryUsage', c_size_t),
]

RKRadarRelay = struct_rk_radar_relay# headers/RadarKit/RKRadarRelay.h: 64

# headers/RadarKit/RKHostMonitor.h: 19
class struct_rk_unit_monitor(Structure):
    pass

RKUnitMonitor = struct_rk_unit_monitor# headers/RadarKit/RKHostMonitor.h: 14

# headers/RadarKit/RKHostMonitor.h: 33
class struct_rk_host_monitor(Structure):
    pass

RKHostMonitor = struct_rk_host_monitor# headers/RadarKit/RKHostMonitor.h: 15

struct_rk_unit_monitor.__slots__ = [
    'name',
    'id',
    'tid',
    'parent',
    'tic',
    'pingIntervalInSeconds',
    'sequenceNumber',
    'identifier',
    'latestTime',
    'hostStatus',
]
struct_rk_unit_monitor._fields_ = [
    ('name', RKShortName),
    ('id', c_int),
    ('tid', pthread_t),
    ('parent', POINTER(RKHostMonitor)),
    ('tic', uint64_t),
    ('pingIntervalInSeconds', uint16_t),
    ('sequenceNumber', uint16_t),
    ('identifier', uint16_t),
    ('latestTime', struct_timeval),
    ('hostStatus', RKHostStatus),
]

struct_rk_host_monitor.__slots__ = [
    'name',
    'verbose',
    'hosts',
    'tic',
    'workerCount',
    'workers',
    'tidHostWatcher',
    'mutex',
    'allKnown',
    'allReachable',
    'anyReachable',
    'state',
    'memoryUsage',
]
struct_rk_host_monitor._fields_ = [
    ('name', RKName),
    ('verbose', uint8_t),
    ('hosts', POINTER(RKName)),
    ('tic', uint64_t),
    ('workerCount', c_int),
    ('workers', POINTER(RKUnitMonitor)),
    ('tidHostWatcher', pthread_t),
    ('mutex', pthread_mutex_t),
    ('allKnown', c_bool),
    ('allReachable', c_bool),
    ('anyReachable', c_bool),
    ('state', RKEngineState),
    ('memoryUsage', uint32_t),
]

RKRadarState = uint32_t# headers/RadarKit/RKRadar.h: 39

# headers/RadarKit/RKRadar.h: 92
class struct_rk_radar(Structure):
    pass

RKRadar = struct_rk_radar# headers/RadarKit/RKRadar.h: 75

# headers/RadarKit/RKRadar.h: 80
class struct_rk_user_device(Structure):
    pass

RKUserDevice = struct_rk_user_device# headers/RadarKit/RKRadar.h: 76

struct_rk_user_device.__slots__ = [
    'device',
    'init',
    'exec',
    'free',
    'initInput',
    'response',
]
struct_rk_user_device._fields_ = [
    ('device', RKHealthRelay),
    ('init', CFUNCTYPE(UNCHECKED(RKHealthRelay), POINTER(RKRadar), POINTER(None))),
    ('exec', CFUNCTYPE(UNCHECKED(c_int), RKHealthRelay, String, String)),
    ('free', CFUNCTYPE(UNCHECKED(c_int), RKHealthRelay)),
    ('initInput', POINTER(None)),
    ('response', c_char * int(4096)),
]

struct_rk_radar.__slots__ = [
    'name',
    'desc',
    'state',
    'active',
    'memoryUsage',
    'processorCount',
    'tic',
    'mutex',
    'status',
    'configs',
    'healths',
    'positions',
    'pulses',
    'rays',
    'statusIndex',
    'configIndex',
    'healthIndex',
    'positionIndex',
    'pulseIndex',
    'rayIndex',
    'productIndex',
    'healthNodeCount',
    'healthNodes',
    'pulseClock',
    'positionClock',
    'fftModule',
    'healthEngine',
    'positionEngine',
    'steerEngine',
    'pulseEngine',
    'pulseRingFilterEngine',
    'momentEngine',
    'rawDataRecorder',
    'healthLogger',
    'sweepEngine',
    'fileManager',
    'radarRelay',
    'hostMonitor',
    'systemInspector',
    'waveform',
    'waveformDecimate',
    'filter',
    'transceiver',
    'transceiverInit',
    'transceiverExec',
    'transceiverFree',
    'transceiverInitInput',
    'transceiverResponse',
    'pedestal',
    'pedestalInit',
    'pedestalExec',
    'pedestalFree',
    'pedestalInitInput',
    'pedestalResponse',
    'healthRelay',
    'healthRelayInit',
    'healthRelayExec',
    'healthRelayFree',
    'healthRelayInitInput',
    'healthRelayResponse',
    'masterController',
    'masterControllerExec',
    'userDevices',
    'waveformCalibrations',
    'waveformCalibrationCount',
    'controls',
    'controlCount',
    'userModule',
    'userModuleInit',
    'userModuleFree',
]
struct_rk_radar._fields_ = [
    ('name', RKName),
    ('desc', RKRadarDesc),
    ('state', RKRadarState),
    ('active', c_bool),
    ('memoryUsage', c_size_t),
    ('processorCount', uint8_t),
    ('tic', uint64_t),
    ('mutex', pthread_mutex_t),
    ('status', POINTER(RKStatus)),
    ('configs', POINTER(RKConfig)),
    ('healths', POINTER(RKHealth)),
    ('positions', POINTER(RKPosition)),
    ('pulses', RKBuffer),
    ('rays', RKBuffer),
    ('statusIndex', uint32_t),
    ('configIndex', uint32_t),
    ('healthIndex', uint32_t),
    ('positionIndex', uint32_t),
    ('pulseIndex', uint32_t),
    ('rayIndex', uint32_t),
    ('productIndex', uint32_t),
    ('healthNodeCount', RKHealthNode),
    ('healthNodes', POINTER(RKNodalHealth)),
    ('pulseClock', POINTER(RKClock)),
    ('positionClock', POINTER(RKClock)),
    ('fftModule', POINTER(RKFFTModule)),
    ('healthEngine', POINTER(RKHealthEngine)),
    ('positionEngine', POINTER(RKPositionEngine)),
    ('steerEngine', POINTER(RKSteerEngine)),
    ('pulseEngine', POINTER(RKPulseEngine)),
    ('pulseRingFilterEngine', POINTER(RKPulseRingFilterEngine)),
    ('momentEngine', POINTER(RKMomentEngine)),
    ('rawDataRecorder', POINTER(RKRawDataRecorder)),
    ('healthLogger', POINTER(RKHealthLogger)),
    ('sweepEngine', POINTER(RKSweepEngine)),
    ('fileManager', POINTER(RKFileManager)),
    ('radarRelay', POINTER(RKRadarRelay)),
    ('hostMonitor', POINTER(RKHostMonitor)),
    ('systemInspector', POINTER(RKSimpleEngine)),
    ('waveform', POINTER(RKWaveform)),
    ('waveformDecimate', POINTER(RKWaveform)),
    ('filter', POINTER(RKIIRFilter)),
    ('transceiver', RKTransceiver),
    ('transceiverInit', CFUNCTYPE(UNCHECKED(RKTransceiver), POINTER(RKRadar), POINTER(None))),
    ('transceiverExec', CFUNCTYPE(UNCHECKED(c_int), RKTransceiver, String, String)),
    ('transceiverFree', CFUNCTYPE(UNCHECKED(c_int), RKTransceiver)),
    ('transceiverInitInput', POINTER(None)),
    ('transceiverResponse', c_char * int(4096)),
    ('pedestal', RKPedestal),
    ('pedestalInit', CFUNCTYPE(UNCHECKED(RKPedestal), POINTER(RKRadar), POINTER(None))),
    ('pedestalExec', CFUNCTYPE(UNCHECKED(c_int), RKPedestal, String, String)),
    ('pedestalFree', CFUNCTYPE(UNCHECKED(c_int), RKPedestal)),
    ('pedestalInitInput', POINTER(None)),
    ('pedestalResponse', c_char * int(4096)),
    ('healthRelay', RKHealthRelay),
    ('healthRelayInit', CFUNCTYPE(UNCHECKED(RKHealthRelay), POINTER(RKRadar), POINTER(None))),
    ('healthRelayExec', CFUNCTYPE(UNCHECKED(c_int), RKHealthRelay, String, String)),
    ('healthRelayFree', CFUNCTYPE(UNCHECKED(c_int), RKHealthRelay)),
    ('healthRelayInitInput', POINTER(None)),
    ('healthRelayResponse', c_char * int(4096)),
    ('masterController', RKMasterController),
    ('masterControllerExec', CFUNCTYPE(UNCHECKED(c_int), RKMasterController, String, String)),
    ('userDevices', RKUserDevice * int(RKHealthNodeCount)),
    ('waveformCalibrations', POINTER(RKWaveformCalibration)),
    ('waveformCalibrationCount', uint32_t),
    ('controls', POINTER(RKControl)),
    ('controlCount', uint32_t),
    ('userModule', RKUserModule),
    ('userModuleInit', CFUNCTYPE(UNCHECKED(RKUserModule), POINTER(RKWaveform))),
    ('userModuleFree', CFUNCTYPE(UNCHECKED(None), RKUserModule)),
]

RKTestFlag = uint8_t# RKTest.h: 18

enum_anon_204 = c_int# RKTest.h: 19

RKTestFlagNone = 0# RKTest.h: 19

RKTestFlagVerbose = 1# RKTest.h: 19

RKTestFlagShowResults = (1 << 1)# RKTest.h: 19

RKTestSIMDFlag = uint8_t# RKTest.h: 25

enum_anon_205 = c_int# RKTest.h: 26

RKTestSIMDFlagNull = 0# RKTest.h: 26

RKTestSIMDFlagShowNumbers = 1# RKTest.h: 26

RKTestSIMDFlagPerformanceTestArithmetic = (1 << 1)# RKTest.h: 26

RKTestSIMDFlagPerformanceTestDuplicate = (1 << 2)# RKTest.h: 26

RKTestSIMDFlagPerformanceTestAll = (RKTestSIMDFlagPerformanceTestArithmetic | RKTestSIMDFlagPerformanceTestDuplicate)# RKTest.h: 26

RKAxisAction = uint8_t# RKTest.h: 34

enum_anon_206 = c_int# RKTest.h: 35

RKAxisActionStop = 0# RKTest.h: 35

RKAxisActionSpeed = (RKAxisActionStop + 1)# RKTest.h: 35

RKAxisActionPosition = (RKAxisActionSpeed + 1)# RKTest.h: 35

# RKTest.h: 71
class struct_rk_test_transceiver(Structure):
    pass

struct_rk_test_transceiver.__slots__ = [
    'name',
    'verbose',
    'counter',
    'sleepInterval',
    'gateCapacity',
    'gateCount',
    'gateSizeMeters',
    'fs',
    'prt',
    'sprt',
    'waveformCache',
    'waveformCacheIndex',
    'customCommand',
    'simFault',
    'transmitting',
    'chunkSize',
    'periodEven',
    'periodOdd',
    'ticEven',
    'ticOdd',
    'playbackFolder',
    'fileHeaderCache',
    'pulseHeaderCache',
    'tidRunLoop',
    'state',
    'radar',
    'memoryUsage',
    'response',
]
struct_rk_test_transceiver._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('counter', c_long),
    ('sleepInterval', c_int),
    ('gateCapacity', c_int),
    ('gateCount', c_int),
    ('gateSizeMeters', c_float),
    ('fs', c_double),
    ('prt', c_double),
    ('sprt', RKByte),
    ('waveformCache', POINTER(RKWaveform) * int(2)),
    ('waveformCacheIndex', c_uint),
    ('customCommand', RKCommand),
    ('simFault', c_bool),
    ('transmitting', c_bool),
    ('chunkSize', c_int),
    ('periodEven', c_double),
    ('periodOdd', c_double),
    ('ticEven', c_long),
    ('ticOdd', c_long),
    ('playbackFolder', c_char * int(768)),
    ('fileHeaderCache', RKFileHeader),
    ('pulseHeaderCache', RKPulseHeader),
    ('tidRunLoop', pthread_t),
    ('state', RKEngineState),
    ('radar', POINTER(RKRadar)),
    ('memoryUsage', c_size_t),
    ('response', c_char * int(4096)),
]

RKTestTransceiver = struct_rk_test_transceiver# RKTest.h: 71

# RKTest.h: 95
class struct_rk_test_pedestal(Structure):
    pass

struct_rk_test_pedestal.__slots__ = [
    'name',
    'verbose',
    'counter',
    'azimuth',
    'speedAzimuth',
    'targetAzimuth',
    'targetSpeedAzimuth',
    'actionAzimuth',
    'elevation',
    'speedElevation',
    'targetElevation',
    'targetSpeedElevation',
    'actionElevation',
    'tidRunLoop',
    'state',
    'radar',
    'memoryUsage',
    'response',
]
struct_rk_test_pedestal._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('counter', c_ulong),
    ('azimuth', c_float),
    ('speedAzimuth', c_float),
    ('targetAzimuth', c_float),
    ('targetSpeedAzimuth', c_float),
    ('actionAzimuth', RKAxisAction),
    ('elevation', c_float),
    ('speedElevation', c_float),
    ('targetElevation', c_float),
    ('targetSpeedElevation', c_float),
    ('actionElevation', RKAxisAction),
    ('tidRunLoop', pthread_t),
    ('state', RKEngineState),
    ('radar', POINTER(RKRadar)),
    ('memoryUsage', c_size_t),
    ('response', c_char * int(4096)),
]

RKTestPedestal = struct_rk_test_pedestal# RKTest.h: 95

# RKTest.h: 107
class struct_rk_test_health_relay(Structure):
    pass

struct_rk_test_health_relay.__slots__ = [
    'name',
    'verbose',
    'counter',
    'tidRunLoop',
    'state',
    'radar',
    'memoryUsage',
    'response',
]
struct_rk_test_health_relay._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('counter', c_long),
    ('tidRunLoop', pthread_t),
    ('state', RKEngineState),
    ('radar', POINTER(RKRadar)),
    ('memoryUsage', c_size_t),
    ('response', c_char * int(4096)),
]

RKTestHealthRelay = struct_rk_test_health_relay# RKTest.h: 107

# RKTest.h: 111
if _libs["radarkit"].has("RKTestByNumberDescription", "cdecl"):
    RKTestByNumberDescription = _libs["radarkit"].get("RKTestByNumberDescription", "cdecl")
    RKTestByNumberDescription.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKTestByNumberDescription.restype = ReturnString
    else:
        RKTestByNumberDescription.restype = String
        RKTestByNumberDescription.errcheck = ReturnString

# RKTest.h: 112
if _libs["radarkit"].has("RKTestByNumber", "cdecl"):
    RKTestByNumber = _libs["radarkit"].get("RKTestByNumber", "cdecl")
    RKTestByNumber.argtypes = [c_int, POINTER(None)]
    RKTestByNumber.restype = None

# RKTest.h: 116
if _libs["radarkit"].has("RKTestShowTypes", "cdecl"):
    RKTestShowTypes = _libs["radarkit"].get("RKTestShowTypes", "cdecl")
    RKTestShowTypes.argtypes = []
    RKTestShowTypes.restype = None

# RKTest.h: 117
if _libs["radarkit"].has("RKTestTerminalColors", "cdecl"):
    RKTestTerminalColors = _libs["radarkit"].get("RKTestTerminalColors", "cdecl")
    RKTestTerminalColors.argtypes = []
    RKTestTerminalColors.restype = None

# RKTest.h: 118
if _libs["radarkit"].has("RKTestPrettyStrings", "cdecl"):
    RKTestPrettyStrings = _libs["radarkit"].get("RKTestPrettyStrings", "cdecl")
    RKTestPrettyStrings.argtypes = []
    RKTestPrettyStrings.restype = None

# RKTest.h: 119
if _libs["radarkit"].has("RKTestBasicMath", "cdecl"):
    RKTestBasicMath = _libs["radarkit"].get("RKTestBasicMath", "cdecl")
    RKTestBasicMath.argtypes = []
    RKTestBasicMath.restype = None

# RKTest.h: 120
if _libs["radarkit"].has("RKTestParseCommaDelimitedValues", "cdecl"):
    RKTestParseCommaDelimitedValues = _libs["radarkit"].get("RKTestParseCommaDelimitedValues", "cdecl")
    RKTestParseCommaDelimitedValues.argtypes = []
    RKTestParseCommaDelimitedValues.restype = None

# RKTest.h: 121
if _libs["radarkit"].has("RKTestParseJSONString", "cdecl"):
    RKTestParseJSONString = _libs["radarkit"].get("RKTestParseJSONString", "cdecl")
    RKTestParseJSONString.argtypes = []
    RKTestParseJSONString.restype = None

# RKTest.h: 122
if _libs["radarkit"].has("RKTestTemperatureToStatus", "cdecl"):
    RKTestTemperatureToStatus = _libs["radarkit"].get("RKTestTemperatureToStatus", "cdecl")
    RKTestTemperatureToStatus.argtypes = []
    RKTestTemperatureToStatus.restype = None

# RKTest.h: 123
if _libs["radarkit"].has("RKTestGetCountry", "cdecl"):
    RKTestGetCountry = _libs["radarkit"].get("RKTestGetCountry", "cdecl")
    RKTestGetCountry.argtypes = []
    RKTestGetCountry.restype = None

# RKTest.h: 124
if _libs["radarkit"].has("RKTestBufferOverviewText", "cdecl"):
    RKTestBufferOverviewText = _libs["radarkit"].get("RKTestBufferOverviewText", "cdecl")
    RKTestBufferOverviewText.argtypes = [String]
    RKTestBufferOverviewText.restype = None

# RKTest.h: 125
if _libs["radarkit"].has("RKTestHealthOverviewText", "cdecl"):
    RKTestHealthOverviewText = _libs["radarkit"].get("RKTestHealthOverviewText", "cdecl")
    RKTestHealthOverviewText.argtypes = [String]
    RKTestHealthOverviewText.restype = None

# RKTest.h: 126
if _libs["radarkit"].has("RKTestReviseLogicalValues", "cdecl"):
    RKTestReviseLogicalValues = _libs["radarkit"].get("RKTestReviseLogicalValues", "cdecl")
    RKTestReviseLogicalValues.argtypes = []
    RKTestReviseLogicalValues.restype = None

# RKTest.h: 127
if _libs["radarkit"].has("RKTestTimeConversion", "cdecl"):
    RKTestTimeConversion = _libs["radarkit"].get("RKTestTimeConversion", "cdecl")
    RKTestTimeConversion.argtypes = []
    RKTestTimeConversion.restype = None

# RKTest.h: 131
if _libs["radarkit"].has("RKTestCountFiles", "cdecl"):
    RKTestCountFiles = _libs["radarkit"].get("RKTestCountFiles", "cdecl")
    RKTestCountFiles.argtypes = [String]
    RKTestCountFiles.restype = None

# RKTest.h: 132
if _libs["radarkit"].has("RKTestPreparePath", "cdecl"):
    RKTestPreparePath = _libs["radarkit"].get("RKTestPreparePath", "cdecl")
    RKTestPreparePath.argtypes = []
    RKTestPreparePath.restype = None

# RKTest.h: 133
if _libs["radarkit"].has("RKTestReadBareRKComplex", "cdecl"):
    RKTestReadBareRKComplex = _libs["radarkit"].get("RKTestReadBareRKComplex", "cdecl")
    RKTestReadBareRKComplex.argtypes = [String]
    RKTestReadBareRKComplex.restype = None

# RKTest.h: 134
if _libs["radarkit"].has("RKTestPreferenceReading", "cdecl"):
    RKTestPreferenceReading = _libs["radarkit"].get("RKTestPreferenceReading", "cdecl")
    RKTestPreferenceReading.argtypes = []
    RKTestPreferenceReading.restype = None

# RKTest.h: 135
if _libs["radarkit"].has("RKTestIQRead", "cdecl"):
    RKTestIQRead = _libs["radarkit"].get("RKTestIQRead", "cdecl")
    RKTestIQRead.argtypes = [String]
    RKTestIQRead.restype = None

# RKTest.h: 136
if _libs["radarkit"].has("RKTestSweepRead", "cdecl"):
    RKTestSweepRead = _libs["radarkit"].get("RKTestSweepRead", "cdecl")
    RKTestSweepRead.argtypes = [String]
    RKTestSweepRead.restype = None

# RKTest.h: 137
if _libs["radarkit"].has("RKTestProductRead", "cdecl"):
    RKTestProductRead = _libs["radarkit"].get("RKTestProductRead", "cdecl")
    RKTestProductRead.argtypes = [String]
    RKTestProductRead.restype = None

# RKTest.h: 138
if _libs["radarkit"].has("RKTestProductCollectionRead", "cdecl"):
    RKTestProductCollectionRead = _libs["radarkit"].get("RKTestProductCollectionRead", "cdecl")
    RKTestProductCollectionRead.argtypes = [String]
    RKTestProductCollectionRead.restype = None

# RKTest.h: 139
if _libs["radarkit"].has("RKTestProductWrite", "cdecl"):
    RKTestProductWrite = _libs["radarkit"].get("RKTestProductWrite", "cdecl")
    RKTestProductWrite.argtypes = []
    RKTestProductWrite.restype = None

# RKTest.h: 140
if _libs["radarkit"].has("RKTestProductWriteFromPlainToSweep", "cdecl"):
    RKTestProductWriteFromPlainToSweep = _libs["radarkit"].get("RKTestProductWriteFromPlainToSweep", "cdecl")
    RKTestProductWriteFromPlainToSweep.argtypes = []
    RKTestProductWriteFromPlainToSweep.restype = None

# RKTest.h: 141
if _libs["radarkit"].has("RKTestProductWriteFromPlainToProduct", "cdecl"):
    RKTestProductWriteFromPlainToProduct = _libs["radarkit"].get("RKTestProductWriteFromPlainToProduct", "cdecl")
    RKTestProductWriteFromPlainToProduct.argtypes = []
    RKTestProductWriteFromPlainToProduct.restype = None

# RKTest.h: 142
if _libs["radarkit"].has("RKTestProductWriteFromWDSS2ToProduct", "cdecl"):
    RKTestProductWriteFromWDSS2ToProduct = _libs["radarkit"].get("RKTestProductWriteFromWDSS2ToProduct", "cdecl")
    RKTestProductWriteFromWDSS2ToProduct.argtypes = [String, c_int]
    RKTestProductWriteFromWDSS2ToProduct.restype = None

# RKTest.h: 146
if _libs["radarkit"].has("RKTestFileManager", "cdecl"):
    RKTestFileManager = _libs["radarkit"].get("RKTestFileManager", "cdecl")
    RKTestFileManager.argtypes = []
    RKTestFileManager.restype = None

# RKTest.h: 147
if _libs["radarkit"].has("RKTestFileMonitor", "cdecl"):
    RKTestFileMonitor = _libs["radarkit"].get("RKTestFileMonitor", "cdecl")
    RKTestFileMonitor.argtypes = []
    RKTestFileMonitor.restype = None

# RKTest.h: 148
if _libs["radarkit"].has("RKTestHostMonitor", "cdecl"):
    RKTestHostMonitor = _libs["radarkit"].get("RKTestHostMonitor", "cdecl")
    RKTestHostMonitor.argtypes = []
    RKTestHostMonitor.restype = None

# RKTest.h: 149
if _libs["radarkit"].has("RKTestInitializingRadar", "cdecl"):
    RKTestInitializingRadar = _libs["radarkit"].get("RKTestInitializingRadar", "cdecl")
    RKTestInitializingRadar.argtypes = []
    RKTestInitializingRadar.restype = None

# RKTest.h: 150
if _libs["radarkit"].has("RKTestWebSocket", "cdecl"):
    RKTestWebSocket = _libs["radarkit"].get("RKTestWebSocket", "cdecl")
    RKTestWebSocket.argtypes = []
    RKTestWebSocket.restype = None

# RKTest.h: 151
if _libs["radarkit"].has("RKTestRadarHub", "cdecl"):
    RKTestRadarHub = _libs["radarkit"].get("RKTestRadarHub", "cdecl")
    RKTestRadarHub.argtypes = []
    RKTestRadarHub.restype = None

# RKTest.h: 152
if _libs["radarkit"].has("RKTestSimplePulseEngine", "cdecl"):
    RKTestSimplePulseEngine = _libs["radarkit"].get("RKTestSimplePulseEngine", "cdecl")
    RKTestSimplePulseEngine.argtypes = [c_int]
    RKTestSimplePulseEngine.restype = None

# RKTest.h: 153
if _libs["radarkit"].has("RKTestSimpleMomentEngine", "cdecl"):
    RKTestSimpleMomentEngine = _libs["radarkit"].get("RKTestSimpleMomentEngine", "cdecl")
    RKTestSimpleMomentEngine.argtypes = [c_int]
    RKTestSimpleMomentEngine.restype = None

# RKTest.h: 157
if _libs["radarkit"].has("RKTestSIMDBasic", "cdecl"):
    RKTestSIMDBasic = _libs["radarkit"].get("RKTestSIMDBasic", "cdecl")
    RKTestSIMDBasic.argtypes = []
    RKTestSIMDBasic.restype = None

# RKTest.h: 158
if _libs["radarkit"].has("RKTestSIMDComplex", "cdecl"):
    RKTestSIMDComplex = _libs["radarkit"].get("RKTestSIMDComplex", "cdecl")
    RKTestSIMDComplex.argtypes = []
    RKTestSIMDComplex.restype = None

# RKTest.h: 159
if _libs["radarkit"].has("RKTestSIMDComparison", "cdecl"):
    RKTestSIMDComparison = _libs["radarkit"].get("RKTestSIMDComparison", "cdecl")
    RKTestSIMDComparison.argtypes = [RKTestSIMDFlag, c_int]
    RKTestSIMDComparison.restype = None

# RKTest.h: 160
if _libs["radarkit"].has("RKTestSIMD", "cdecl"):
    RKTestSIMD = _libs["radarkit"].get("RKTestSIMD", "cdecl")
    RKTestSIMD.argtypes = [RKTestSIMDFlag, c_int]
    RKTestSIMD.restype = None

# RKTest.h: 161
if _libs["radarkit"].has("RKTestWindow", "cdecl"):
    RKTestWindow = _libs["radarkit"].get("RKTestWindow", "cdecl")
    RKTestWindow.argtypes = [c_int]
    RKTestWindow.restype = None

# RKTest.h: 162
if _libs["radarkit"].has("RKTestHilbertTransform", "cdecl"):
    RKTestHilbertTransform = _libs["radarkit"].get("RKTestHilbertTransform", "cdecl")
    RKTestHilbertTransform.argtypes = []
    RKTestHilbertTransform.restype = None

# RKTest.h: 163
if _libs["radarkit"].has("RKTestWriteFFTWisdom", "cdecl"):
    RKTestWriteFFTWisdom = _libs["radarkit"].get("RKTestWriteFFTWisdom", "cdecl")
    RKTestWriteFFTWisdom.argtypes = [c_int]
    RKTestWriteFFTWisdom.restype = None

# RKTest.h: 164
if _libs["radarkit"].has("RKTestRingFilterShowCoefficients", "cdecl"):
    RKTestRingFilterShowCoefficients = _libs["radarkit"].get("RKTestRingFilterShowCoefficients", "cdecl")
    RKTestRingFilterShowCoefficients.argtypes = []
    RKTestRingFilterShowCoefficients.restype = None

# RKTest.h: 165
if _libs["radarkit"].has("RKTestRamp", "cdecl"):
    RKTestRamp = _libs["radarkit"].get("RKTestRamp", "cdecl")
    RKTestRamp.argtypes = []
    RKTestRamp.restype = None

# RKTest.h: 166
if _libs["radarkit"].has("RKTestMakeHops", "cdecl"):
    RKTestMakeHops = _libs["radarkit"].get("RKTestMakeHops", "cdecl")
    RKTestMakeHops.argtypes = []
    RKTestMakeHops.restype = None

# RKTest.h: 167
if _libs["radarkit"].has("RKTestWaveformTFM", "cdecl"):
    RKTestWaveformTFM = _libs["radarkit"].get("RKTestWaveformTFM", "cdecl")
    RKTestWaveformTFM.argtypes = []
    RKTestWaveformTFM.restype = None

# RKTest.h: 168
if _libs["radarkit"].has("RKTestWaveformWrite", "cdecl"):
    RKTestWaveformWrite = _libs["radarkit"].get("RKTestWaveformWrite", "cdecl")
    RKTestWaveformWrite.argtypes = []
    RKTestWaveformWrite.restype = None

# RKTest.h: 169
if _libs["radarkit"].has("RKTestWaveformDownsampling", "cdecl"):
    RKTestWaveformDownsampling = _libs["radarkit"].get("RKTestWaveformDownsampling", "cdecl")
    RKTestWaveformDownsampling.argtypes = []
    RKTestWaveformDownsampling.restype = None

# RKTest.h: 170
if _libs["radarkit"].has("RKTestWaveformShowProperties", "cdecl"):
    RKTestWaveformShowProperties = _libs["radarkit"].get("RKTestWaveformShowProperties", "cdecl")
    RKTestWaveformShowProperties.argtypes = []
    RKTestWaveformShowProperties.restype = None

# RKTest.h: 171
if _libs["radarkit"].has("RKTestWaveformShowUserWaveformProperties", "cdecl"):
    RKTestWaveformShowUserWaveformProperties = _libs["radarkit"].get("RKTestWaveformShowUserWaveformProperties", "cdecl")
    RKTestWaveformShowUserWaveformProperties.argtypes = [String]
    RKTestWaveformShowUserWaveformProperties.restype = None

# RKTest.h: 175
if _libs["radarkit"].has("RKTestHalfSingleDoubleConversion", "cdecl"):
    RKTestHalfSingleDoubleConversion = _libs["radarkit"].get("RKTestHalfSingleDoubleConversion", "cdecl")
    RKTestHalfSingleDoubleConversion.argtypes = []
    RKTestHalfSingleDoubleConversion.restype = None

# RKTest.h: 176
if _libs["radarkit"].has("RKTestPulseCompression", "cdecl"):
    RKTestPulseCompression = _libs["radarkit"].get("RKTestPulseCompression", "cdecl")
    RKTestPulseCompression.argtypes = [RKTestFlag]
    RKTestPulseCompression.restype = None

# RKTest.h: 177
if _libs["radarkit"].has("RKTestOnePulse", "cdecl"):
    RKTestOnePulse = _libs["radarkit"].get("RKTestOnePulse", "cdecl")
    RKTestOnePulse.argtypes = []
    RKTestOnePulse.restype = None

# RKTest.h: 178
if _libs["radarkit"].has("RKTestOneRay", "cdecl"):
    RKTestOneRay = _libs["radarkit"].get("RKTestOneRay", "cdecl")
    RKTestOneRay.argtypes = [CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t), c_int]
    RKTestOneRay.restype = None

# RKTest.h: 179
if _libs["radarkit"].has("RKTestOneRaySpectra", "cdecl"):
    RKTestOneRaySpectra = _libs["radarkit"].get("RKTestOneRaySpectra", "cdecl")
    RKTestOneRaySpectra.argtypes = [CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t), c_int]
    RKTestOneRaySpectra.restype = None

# RKTest.h: 183
if _libs["radarkit"].has("RKTestPulseCompressionSpeed", "cdecl"):
    RKTestPulseCompressionSpeed = _libs["radarkit"].get("RKTestPulseCompressionSpeed", "cdecl")
    RKTestPulseCompressionSpeed.argtypes = [c_int]
    RKTestPulseCompressionSpeed.restype = None

# RKTest.h: 184
if _libs["radarkit"].has("RKTestPulseEngineSpeed", "cdecl"):
    RKTestPulseEngineSpeed = _libs["radarkit"].get("RKTestPulseEngineSpeed", "cdecl")
    RKTestPulseEngineSpeed.argtypes = [c_int]
    RKTestPulseEngineSpeed.restype = None

# RKTest.h: 185
if _libs["radarkit"].has("RKTestMomentProcessorSpeed", "cdecl"):
    RKTestMomentProcessorSpeed = _libs["radarkit"].get("RKTestMomentProcessorSpeed", "cdecl")
    RKTestMomentProcessorSpeed.argtypes = []
    RKTestMomentProcessorSpeed.restype = None

# RKTest.h: 186
if _libs["radarkit"].has("RKTestCacheWrite", "cdecl"):
    RKTestCacheWrite = _libs["radarkit"].get("RKTestCacheWrite", "cdecl")
    RKTestCacheWrite.argtypes = []
    RKTestCacheWrite.restype = None

# RKTest.h: 190
if _libs["radarkit"].has("RKTestTransceiverInit", "cdecl"):
    RKTestTransceiverInit = _libs["radarkit"].get("RKTestTransceiverInit", "cdecl")
    RKTestTransceiverInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestTransceiverInit.restype = RKTransceiver

# RKTest.h: 191
if _libs["radarkit"].has("RKTestTransceiverExec", "cdecl"):
    RKTestTransceiverExec = _libs["radarkit"].get("RKTestTransceiverExec", "cdecl")
    RKTestTransceiverExec.argtypes = [RKTransceiver, String, String]
    RKTestTransceiverExec.restype = c_int

# RKTest.h: 192
if _libs["radarkit"].has("RKTestTransceiverFree", "cdecl"):
    RKTestTransceiverFree = _libs["radarkit"].get("RKTestTransceiverFree", "cdecl")
    RKTestTransceiverFree.argtypes = [RKTransceiver]
    RKTestTransceiverFree.restype = c_int

# RKTest.h: 196
if _libs["radarkit"].has("RKTestPedestalInit", "cdecl"):
    RKTestPedestalInit = _libs["radarkit"].get("RKTestPedestalInit", "cdecl")
    RKTestPedestalInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestPedestalInit.restype = RKPedestal

# RKTest.h: 197
if _libs["radarkit"].has("RKTestPedestalExec", "cdecl"):
    RKTestPedestalExec = _libs["radarkit"].get("RKTestPedestalExec", "cdecl")
    RKTestPedestalExec.argtypes = [RKPedestal, String, String]
    RKTestPedestalExec.restype = c_int

# RKTest.h: 198
if _libs["radarkit"].has("RKTestPedestalFree", "cdecl"):
    RKTestPedestalFree = _libs["radarkit"].get("RKTestPedestalFree", "cdecl")
    RKTestPedestalFree.argtypes = [RKPedestal]
    RKTestPedestalFree.restype = c_int

# RKTest.h: 202
if _libs["radarkit"].has("RKTestHealthRelayInit", "cdecl"):
    RKTestHealthRelayInit = _libs["radarkit"].get("RKTestHealthRelayInit", "cdecl")
    RKTestHealthRelayInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestHealthRelayInit.restype = RKHealthRelay

# RKTest.h: 203
if _libs["radarkit"].has("RKTestHealthRelayExec", "cdecl"):
    RKTestHealthRelayExec = _libs["radarkit"].get("RKTestHealthRelayExec", "cdecl")
    RKTestHealthRelayExec.argtypes = [RKHealthRelay, String, String]
    RKTestHealthRelayExec.restype = c_int

# RKTest.h: 204
if _libs["radarkit"].has("RKTestHealthRelayFree", "cdecl"):
    RKTestHealthRelayFree = _libs["radarkit"].get("RKTestHealthRelayFree", "cdecl")
    RKTestHealthRelayFree.argtypes = [RKHealthRelay]
    RKTestHealthRelayFree.restype = c_int

# RKTest.h: 208
if _libs["radarkit"].has("RKTestProductWriteFromPlainToProduct", "cdecl"):
    RKTestProductWriteFromPlainToProduct = _libs["radarkit"].get("RKTestProductWriteFromPlainToProduct", "cdecl")
    RKTestProductWriteFromPlainToProduct.argtypes = []
    RKTestProductWriteFromPlainToProduct.restype = None

# RKTest.h: 212
if _libs["radarkit"].has("RKTestCommandQueue", "cdecl"):
    RKTestCommandQueue = _libs["radarkit"].get("RKTestCommandQueue", "cdecl")
    RKTestCommandQueue.argtypes = []
    RKTestCommandQueue.restype = None

# RKTest.h: 213
if _libs["radarkit"].has("RKTestSingleCommand", "cdecl"):
    RKTestSingleCommand = _libs["radarkit"].get("RKTestSingleCommand", "cdecl")
    RKTestSingleCommand.argtypes = []
    RKTestSingleCommand.restype = None

# RKTest.h: 214
if _libs["radarkit"].has("RKTestExperiment", "cdecl"):
    RKTestExperiment = _libs["radarkit"].get("RKTestExperiment", "cdecl")
    RKTestExperiment.argtypes = [String]
    RKTestExperiment.restype = None

# RKTypes.h: 11
try:
    __STDC_WANT_LIB_EXT1__ = 1
except:
    pass

# /usr/include/math.h: 1151
try:
    M_PI = 3.14159265358979323846
except:
    pass

# /usr/include/x86_64-linux-gnu/sys/param.h: 102
def MIN(a, b):
    return (a < b) and a or b

# /usr/include/x86_64-linux-gnu/sys/param.h: 103
def MAX(a, b):
    return (a > b) and a or b

# /usr/include/asm-generic/errno-base.h: 8
try:
    EINTR = 4
except:
    pass

# /usr/include/asm-generic/errno-base.h: 9
try:
    EIO = 5
except:
    pass

# /usr/include/asm-generic/errno-base.h: 13
try:
    EBADF = 9
except:
    pass

# /usr/include/asm-generic/errno-base.h: 15
try:
    EAGAIN = 11
except:
    pass

# /usr/include/asm-generic/errno-base.h: 17
try:
    EACCES = 13
except:
    pass

# /usr/include/asm-generic/errno-base.h: 18
try:
    EFAULT = 14
except:
    pass

# /usr/include/asm-generic/errno-base.h: 26
try:
    EINVAL = 22
except:
    pass

# /usr/include/asm-generic/errno.h: 94
try:
    ECONNREFUSED = 111
except:
    pass

# /usr/include/asm-generic/errno.h: 95
try:
    EHOSTDOWN = 112
except:
    pass

# /usr/include/asm-generic/errno.h: 96
try:
    EHOSTUNREACH = 113
except:
    pass

# /usr/include/errno.h: 38
try:
    errno = ((__errno_location ())[0])
except:
    pass

# RKTypes.h: 59
try:
    RKRawDataFormat = 8
except:
    pass

# RKTypes.h: 60
try:
    RKBufferSSlotCount = 10
except:
    pass

# RKTypes.h: 61
try:
    RKBufferCSlotCount = 10
except:
    pass

# RKTypes.h: 62
try:
    RKBufferHSlotCount = 50
except:
    pass

# RKTypes.h: 63
try:
    RKBufferPSlotCount = 1000
except:
    pass

# RKTypes.h: 64
try:
    RKBuffer0SlotCount = 20000
except:
    pass

# RKTypes.h: 65
try:
    RKBuffer2SlotCount = 3600
except:
    pass

# RKTypes.h: 66
try:
    RKBuffer3SlotCount = 100
except:
    pass

# RKTypes.h: 67
try:
    RKMaximumControlCount = 128
except:
    pass

# RKTypes.h: 68
try:
    RKMaximumWaveformCalibrationCount = 128
except:
    pass

# RKTypes.h: 69
try:
    RKMaximumGateCount = 262144
except:
    pass

# RKTypes.h: 70
try:
    RKMemoryAlignSize = 64
except:
    pass

# RKTypes.h: 71
try:
    RKBaseProductCount = 19
except:
    pass

# RKTypes.h: 72
try:
    RKMaximumLagCount = 5
except:
    pass

# RKTypes.h: 73
try:
    RKMaximumFilterCount = 8
except:
    pass

# RKTypes.h: 74
try:
    RKMaximumWaveformCount = 22
except:
    pass

# RKTypes.h: 75
try:
    RKWorkerDutyCycleBufferDepth = 1000
except:
    pass

# RKTypes.h: 76
try:
    RKMaximumPulsesPerRay = 2000
except:
    pass

# RKTypes.h: 77
try:
    RKMaximumRaysPerSweep = 1500
except:
    pass

# RKTypes.h: 78
try:
    RKMaximumPacketSize = ((16 * 1024) * 1024)
except:
    pass

# RKTypes.h: 79
try:
    RKNetworkTimeoutSeconds = 20
except:
    pass

# RKTypes.h: 80
try:
    RKNetworkReconnectSeconds = 3
except:
    pass

# RKTypes.h: 81
try:
    RKLagRedThreshold = 0.5
except:
    pass

# RKTypes.h: 82
try:
    RKLagOrangeThreshold = 0.7
except:
    pass

# RKTypes.h: 83
try:
    RKDutyCyleRedThreshold = 0.9
except:
    pass

# RKTypes.h: 84
try:
    RKDutyCyleOrangeThreshold = 0.8
except:
    pass

# RKTypes.h: 85
try:
    RKStatusBarWidth = 6
except:
    pass

# RKTypes.h: 86
try:
    RKPulseCountForNoiseMeasurement = 200
except:
    pass

# RKTypes.h: 87
try:
    RKProcessorStatusPulseCoreCount = 16
except:
    pass

# RKTypes.h: 88
try:
    RKProcessorStatusRingCoreCount = 16
except:
    pass

# RKTypes.h: 89
try:
    RKProcessorStatusRayCoreCount = 16
except:
    pass

# RKTypes.h: 90
try:
    RKHostMonitorPingInterval = 5
except:
    pass

# RKTypes.h: 91
try:
    RKMaximumProductCount = 64
except:
    pass

# RKTypes.h: 92
try:
    RKMaximumIIRFilterTaps = 8
except:
    pass

# RKTypes.h: 93
try:
    RKMaximumPrefixLength = 8
except:
    pass

# RKTypes.h: 94
try:
    RKMaximumSymbolLength = 8
except:
    pass

# RKTypes.h: 95
try:
    RKMaximumFileExtensionLength = 8
except:
    pass

# RKTypes.h: 96
try:
    RKUserParameterCount = 8
except:
    pass

# RKTypes.h: 97
try:
    RKMaximumScanCount = 256
except:
    pass

# RKTypes.h: 98
try:
    RKPedestalActionBufferDepth = 8
except:
    pass

# RKTypes.h: 99
try:
    RKDefaultScanSpeed = 18.0
except:
    pass

# RKTypes.h: 101
try:
    RKDefaultDataPath = 'data'
except:
    pass

# RKTypes.h: 102
try:
    RKDataFolderIQ = 'iq'
except:
    pass

# RKTypes.h: 103
try:
    RKDataFolderMoment = 'moment'
except:
    pass

# RKTypes.h: 104
try:
    RKDataFolderHealth = 'health'
except:
    pass

# RKTypes.h: 105
try:
    RKLogFolder = 'log'
except:
    pass

# RKTypes.h: 106
try:
    RKWaveformFolder = 'waveform'
except:
    pass

# RKTypes.h: 107
try:
    RKFFTWisdomFile = 'radarkit-fft-wisdom'
except:
    pass

# RKTypes.h: 109
try:
    RKNoColor = '\\033[0m'
except:
    pass

# RKTypes.h: 110
try:
    RKNoForegroundColor = '\\033[39m'
except:
    pass

# RKTypes.h: 111
try:
    RKNoBackgroundColor = '\\033[49m'
except:
    pass

# RKTypes.h: 112
try:
    RKBaseRedColor = '\\033[91m'
except:
    pass

# RKTypes.h: 113
try:
    RKBaseGreenColor = '\\033[92m'
except:
    pass

# RKTypes.h: 114
try:
    RKBaseYellowColor = '\\033[93m'
except:
    pass

# RKTypes.h: 115
try:
    RKRedColor = '\\033[38;5;196m'
except:
    pass

# RKTypes.h: 116
try:
    RKOrangeColor = '\\033[38;5;208m'
except:
    pass

# RKTypes.h: 117
try:
    RKYellowColor = '\\033[38;5;226m'
except:
    pass

# RKTypes.h: 118
try:
    RKCreamColor = '\\033[38;5;229m'
except:
    pass

# RKTypes.h: 119
try:
    RKGoldColor = '\\033[38;5;178m'
except:
    pass

# RKTypes.h: 120
try:
    RKLimeColor = '\\033[38;5;118m'
except:
    pass

# RKTypes.h: 121
try:
    RKMintColor = '\\033[38;5;43m'
except:
    pass

# RKTypes.h: 122
try:
    RKGreenColor = '\\033[38;5;46m'
except:
    pass

# RKTypes.h: 123
try:
    RKTealColor = '\\033[38;5;49m'
except:
    pass

# RKTypes.h: 124
try:
    RKIceBlueColor = '\\033[38;5;51m'
except:
    pass

# RKTypes.h: 125
try:
    RKSkyBlueColor = '\\033[38;5;45m'
except:
    pass

# RKTypes.h: 126
try:
    RKBlueColor = '\\033[38;5;27m'
except:
    pass

# RKTypes.h: 127
try:
    RKPurpleColor = '\\033[38;5;99m'
except:
    pass

# RKTypes.h: 128
try:
    RKIndigoColor = '\\033[38;5;201m'
except:
    pass

# RKTypes.h: 129
try:
    RKHotPinkColor = '\\033[38;5;199m'
except:
    pass

# RKTypes.h: 130
try:
    RKDeepPinkColor = '\\033[38;5;197m'
except:
    pass

# RKTypes.h: 131
try:
    RKPinkColor = '\\033[38;5;213m'
except:
    pass

# RKTypes.h: 132
try:
    RKSalmonColor = '\\033[38;5;210m'
except:
    pass

# RKTypes.h: 133
try:
    RKGrayColor = '\\033[38;5;245m'
except:
    pass

# RKTypes.h: 134
try:
    RKWhiteColor = '\\033[38;5;15m'
except:
    pass

# RKTypes.h: 135
try:
    RKMonokaiRed = '\\033[38;5;196m'
except:
    pass

# RKTypes.h: 136
try:
    RKMonokaiPink = '\\033[38;5;197m'
except:
    pass

# RKTypes.h: 137
try:
    RKMonokaiOrange = '\\033[38;5;208m'
except:
    pass

# RKTypes.h: 138
try:
    RKMonokaiYellow = '\\033[38;5;186m'
except:
    pass

# RKTypes.h: 139
try:
    RKMonokaiGreen = '\\033[38;5;154m'
except:
    pass

# RKTypes.h: 140
try:
    RKMonokaiBlue = '\\033[38;5;81m'
except:
    pass

# RKTypes.h: 141
try:
    RKMonokaiPurple = '\\033[38;5;141m'
except:
    pass

# RKTypes.h: 142
try:
    RKLightOrangeColor = '\\033[38;5;214m'
except:
    pass

# RKTypes.h: 143
try:
    RKWarningColor = '\\033[38;5;15;48;5;197m'
except:
    pass

# RKTypes.h: 144
try:
    RKPythonColor = '\\033[38;5;226;48;5;24m'
except:
    pass

# RKTypes.h: 145
try:
    RKRadarKitColor = '\\033[38;5;15;48;5;124m'
except:
    pass

# RKTypes.h: 146
try:
    RKMaximumStringLength = 4096
except:
    pass

# RKTypes.h: 147
try:
    RKMaximumPathLength = 1024
except:
    pass

# RKTypes.h: 148
try:
    RKMaximumFolderPathLength = 768
except:
    pass

# RKTypes.h: 149
try:
    RKMaximumCommandLength = 512
except:
    pass

# RKTypes.h: 150
try:
    RKStatusStringLength = 256
except:
    pass

# RKTypes.h: 151
try:
    RKPulseHeaderPaddedSize = 384
except:
    pass

# RKTypes.h: 152
try:
    RKRayHeaderPaddedSize = 128
except:
    pass

# RKTypes.h: 153
try:
    RKNameLength = 128
except:
    pass

# RKTypes.h: 154
try:
    RKShortNameLength = 20
except:
    pass

# RKTypes.h: 155
try:
    RKChildNameLength = 160
except:
    pass

# RKTypes.h: 157
def RKColorDutyCycle(x):
    return (x > RKDutyCyleRedThreshold) and RKBaseRedColor or (x > RKDutyCyleOrangeThreshold) and RKBaseYellowColor or RKBaseGreenColor

# RKTypes.h: 158
def RKColorLag(x):
    return (x > RKLagRedThreshold) and RKBaseRedColor or (x > RKLagOrangeThreshold) and RKBaseYellowColor or RKBaseGreenColor

# RKTypes.h: 160
def RKDigitWidth(v, n):
    return (c_int (ord_if_char((((floorf ((log10f ((fabsf (v)))))) + (v < 0)) + n and (n + 2) or 1)))).value

# RKTypes.h: 162
def ITALIC(x):
    return (('\\033[3m' + x) + '\\033[23m')

# RKTypes.h: 163
def UNDERLINE(x):
    return (('\\033[4m' + x) + '\\033[24m')

# RKTypes.h: 164
def HIGHLIGHT(x):
    return (('\\033[38;5;82;48;5;238m' + x) + '\\033[m')

# RKTypes.h: 165
def UNDERLINE_ITALIC(x):
    return (('\\033[3;4m' + x) + '\\033[23;24m')

# RKTypes.h: 171
def RKMarkerScanTypeString(x):
    return ((x & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) and 'PPI' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) and 'RHI' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint) and 'SPT' or 'UNK'

# RKTypes.h: 176
def RKMarkerScanTypeShortString(x):
    return ((x & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) and 'P' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) and 'R' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint) and 'S' or 'U'

# RKTypes.h: 181
def RKPositionAzimuthFlagColor(x):
    return (x & RKPositionFlagAzimuthError) and RKRedColor or (x & RKPositionFlagAzimuthEnabled) and RKGreenColor or RKYellowColor

# RKTypes.h: 185
def RKPositionElevationFlagColor(x):
    return (x & RKPositionFlagElevationError) and RKRedColor or (x & RKPositionFlagElevationEnabled) and RKGreenColor or RKYellowColor

# RKMisc.h: 48
def RKErrnoString(A):
    return (errno == EAGAIN) and 'EAGAIN' or (errno == EBADF) and 'EBADF' or (errno == EFAULT) and 'EFAULT' or (errno == EINTR) and 'EINTR' or (errno == EINVAL) and 'EINVAL' or (errno == ECONNREFUSED) and 'ECONNREFUSED' or (errno == EHOSTDOWN) and 'EHOSTDOWN' or (errno == EHOSTUNREACH) and 'EHOSTUNREACH' or (errno == EACCES) and 'EACCES' or (errno == EIO) and 'EIO' or 'OTHERS'

# RKMisc.h: 87
def CLAMP(x, lo, hi):
    return (MIN ((MAX (x, lo)), hi))

# RKMisc.h: 111
try:
    RKMiscStringLength = 1024
except:
    pass

# RKFoundation.h: 17
try:
    RKDefaultLogfile = 'messages.log'
except:
    pass

# RKFoundation.h: 18
try:
    RKEOL = '\\r\\n'
except:
    pass

# RKFoundation.h: 21
def RKNextNModuloS(i, N, S):
    return (i >= (S - N)) and ((i + N) - S) or (i + N)

# RKFoundation.h: 22
def RKPreviousNModuloS(i, N, S):
    return (i < N) and ((S + i) - N) or (i - N)

# RKFoundation.h: 25
def RKNextModuloS(i, S):
    return (i == (S - 1)) and 0 or (i + 1)

# RKFoundation.h: 26
def RKPreviousModuloS(i, S):
    return (i == 0) and (S - 1) or (i - 1)

# RKFoundation.h: 29
def RKModuloLag(h, t, S):
    return (h < t) and ((h + S) - t) or (h - t)

# RKFoundation.h: 67
def RKRho2Uint8(r):
    return (roundf ((r > 0.93) and ((r * 1000.0) - 824.0) or (r > 0.7) and ((r * 300.0) - 173.0) or (r * 52.8571)))

# RKFoundation.h: 69
def RKSingleWrapTo2PI(x):
    return (x < (-M_PI)) and (x + (2.0 * M_PI)) or (x >= M_PI) and (x - (2.0 * M_PI)) or x

# RKFoundation.h: 71
def RKInstructIsAzimuth(i):
    return ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisAzimuth)

# RKFoundation.h: 72
def RKInstructIsElevation(i):
    return ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisElevation)

# RKFoundation.h: 73
def RKInstructIsNone(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeNone)

# RKFoundation.h: 74
def RKInstructIsTest(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeTest)

# RKFoundation.h: 75
def RKInstructIsSlew(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeSlew)

# RKFoundation.h: 76
def RKInstructIsPoint(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModePoint)

# RKFoundation.h: 77
def RKInstructIsStandby(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeStandby)

# RKFoundation.h: 78
def RKInstructIsDisable(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeDisable)

# RKFoundation.h: 79
def RKInstructIsEnable(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeEnable)

# headers/RadarKit/RKWaveform.h: 15
try:
    RKWaveformDefaultDepth = 1024
except:
    pass

# headers/RadarKit/RKWaveform.h: 16
try:
    RKWaveformDigitalAmplitude = 32000.0
except:
    pass

# RKRawDataRecorder.h: 16
try:
    RKRawDataRecorderDefaultMaximumRecorderDepth = 100000
except:
    pass

# RKRawDataRecorder.h: 17
try:
    RKRawDataRecorderDefaultCacheSize = ((32 * 1024) * 1024)
except:
    pass

# RKMomentEngine.h: 21
try:
    RKMomentDFTPlanCount = 16
except:
    pass

# RKSweepEngine.h: 12
try:
    RKSweepScratchSpaceDepth = 6
except:
    pass

# RKSweepEngine.h: 13
try:
    RKMaximumProductBufferDepth = 20
except:
    pass

# RKSweepEngine.h: 14
try:
    RKMaximumListLength = (RKMaximumPathLength + (RKMaximumProductBufferDepth * RKMaximumPathLength))
except:
    pass

# /usr/include/netcdf.h: 129
try:
    NC_CLOBBER = 0x0000
except:
    pass

# headers/RadarKit/RKProductFile.h: 22
try:
    W2_MISSING_DATA = (-99900.0)
except:
    pass

# headers/RadarKit/RKProductFile.h: 23
try:
    W2_RANGE_FOLDED = (-99901.0)
except:
    pass

# headers/RadarKit/RKProductFile.h: 28
try:
    NC_MODE = NC_CLOBBER
except:
    pass

# RKTest.h: 16
try:
    RKTestWaveformCacheCount = 2
except:
    pass

rk_int16c = struct_rk_int16c# RKTypes.h: 222

rk_complex = struct_rk_complex# RKTypes.h: 230

rk_iqz = struct_rk_iqz# RKTypes.h: 238

rk_modulo_path = struct_rk_modulo_path# RKTypes.h: 248

rk_four_byte = union_rk_four_byte# RKTypes.h: 262

rk_half_float_t = union_rk_half_float_t# RKTypes.h: 275

rk_single_float_t = union_rk_single_float_t# RKTypes.h: 289

rk_double_float_t = union_rk_double_float_t# RKTypes.h: 303

rk_filter_anchor = union_rk_filter_anchor# RKTypes.h: 321

rk_radarhub_ray_header = union_rk_radarhub_ray_header# RKTypes.h: 1202

rk_radarhub_ray = union_rk_radarhub_ray# RKTypes.h: 1210

rk_radar_desc = struct_rk_radar_desc# RKTypes.h: 1255

rk_waveform = struct_rk_waveform# RKTypes.h: 1268

rk_wave_file_header = union_rk_wave_file_header# RKTypes.h: 1281

rk_waveform_cal = struct_rk_waveform_cal# RKTypes.h: 1290

rk_waveform_response = struct_rk_waveform_response# RKTypes.h: 1297

rk_config = union_rk_config# RKTypes.h: 1336

rk_health = union_rk_health# RKTypes.h: 1350

rk_nodal_health = struct_rk_nodal_health# RKTypes.h: 1359

rk_position = union_rk_position# RKTypes.h: 1395

rk_scan_action = struct_rk_scan_action# RKTypes.h: 1402

rk_pulse_header = union_rk_pulse_header# RKTypes.h: 1430

rk_pulse_parameters = struct_rk_pulse_parameters# RKTypes.h: 1440

rk_pulse = struct_rk_pulse# RKTypes.h: 1456

rk_ray_header = struct_rk_ray_header# RKTypes.h: 1488

rk_ray = struct_rk_ray# RKTypes.h: 1501

rk_sweep_header = struct_rk_sweep_header# RKTypes.h: 1521

rk_sweep = struct_rk_sweep# RKTypes.h: 1530

rk_file_header = union_rk_file_header# RKTypes.h: 1542

rk_preferene_object = struct_rk_preferene_object# RKTypes.h: 1556

rk_control = struct_rk_control# RKTypes.h: 1567

rk_status = struct_rk_status# RKTypes.h: 1591

rk_simple_engine = struct_rk_simple_engine# RKTypes.h: 1605

rk_file_monitor = struct_rk_file_monitor# RKTypes.h: 1619

rk_product_desc = union_rk_product_desc# RKTypes.h: 1645

rk_product_header = union_rk_product_header# RKTypes.h: 1684

rk_product = struct_rk_product# RKTypes.h: 1701

rk_product_collection = struct_rk_product_collection# RKTypes.h: 1706

rk_iir_filter = struct_rk_iir_filter# RKTypes.h: 1715

rk_task = struct_rk_task# RKTypes.h: 1721

rk_command_queue = struct_rk_command_queue# RKTypes.h: 1733

RKGlobalParameterStruct = struct_RKGlobalParameterStruct# RKFoundation.h: 93

rk_fft_resource = struct_rk_fft_resource# RKDSP.h: 25

rk_fft_module = struct_rk_fft_module# RKDSP.h: 34

rk_gaussian = struct_rk_gaussian# RKDSP.h: 40

rk_compression_scratch = struct_rk_compression_scratch# headers/RadarKit/RKScratch.h: 52

rk_moment_scratch = struct_rk_moment_scratch# headers/RadarKit/RKScratch.h: 112

rk_pulse_worker = struct_rk_pulse_worker# RKPulseEngine.h: 22

rk_pulse_engine = struct_rk_pulse_engine# RKPulseEngine.h: 40

rk_data_recorder = struct_rk_data_recorder# RKRawDataRecorder.h: 21

rk_moment_worker = struct_rk_moment_worker# RKMomentEngine.h: 26

rk_moment_engine = struct_rk_moment_engine# RKMomentEngine.h: 41

rk_sweep_scratch = struct_rk_sweep_scratch# RKSweepEngine.h: 27

rk_sweep_engine = struct_rk_sweep_engine# RKSweepEngine.h: 31

rk_pulse_ring_filter_worker = struct_rk_pulse_ring_filter_worker# RKPulseRingFilter.h: 21

rk_pulse_ring_filter_engine = struct_rk_pulse_ring_filter_engine# RKPulseRingFilter.h: 39

rk_test_transceiver = struct_rk_test_transceiver# RKTest.h: 71

rk_test_pedestal = struct_rk_test_pedestal# RKTest.h: 95

rk_test_health_relay = struct_rk_test_health_relay# RKTest.h: 107

# No inserted files

# No prefix-stripping

