r"""Wrapper for RKTypes.h

Generated with:
/home/tmin/.local/bin/ctypesgen -I/usr/local/include -Iheaders -Iheaders/RadarKit -L./ -lradarkit headers/RadarKit/RKTypes.h headers/RadarKit/RKMisc.h headers/RadarKit/RKFoundation.h headers/RadarKit/RKConfig.h headers/RadarKit/RKDSP.h headers/RadarKit/RKPulseEngine.h headers/RadarKit/RKFileHeader.h headers/RadarKit/RKScratch.h headers/RadarKit/RKRawDataRecorder.h headers/RadarKit/RKMomentEngine.h headers/RadarKit/RKNoiseEstimator.h headers/RadarKit/RKSweepEngine.h headers/RadarKit/RKPulseRingFilter.h headers/RadarKit/RKMultiLag.h headers/RadarKit/RKPulseATSR.h headers/RadarKit/RKWaveform.h headers/RadarKit.h headers/RadarKit/RKTest.h -o python/radarkit/_ctypes_.py

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
_libdirs = ['./']

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

add_library_search_dirs(['./'])

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

time_t = __time_t# /usr/include/x86_64-linux-gnu/bits/types/time_t.h: 10

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

fftwf_complex = c_float * int(2)# /usr/include/fftw3.h: 458

# /usr/include/fftw3.h: 458
class struct_fftwf_plan_s(Structure):
    pass

fftwf_plan = POINTER(struct_fftwf_plan_s)# /usr/include/fftw3.h: 458

RKByte = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 194

RKFloat = c_float# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 195

RKResult = c_ptrdiff_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 196

RKBuffer = POINTER(uint8_t)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 197

RKTransceiver = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 198

RKPedestal = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 199

RKHealthRelay = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 200

RKMasterController = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 201

RKUserResource = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 202

RKUserModule = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 203

RKCompressor = POINTER(None)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 204

RKName = c_char * int(128)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 205

RKChildName = c_char * int(160)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 206

RKShortName = c_char * int(20)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 207

RKCommand = c_char * int(512)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 208

RKProductId = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 209

RKIdentifier = uint64_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 210

RKConst = c_float# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 211

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 225
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

RKInt16C = struct_rk_int16c# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 225

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 233
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

RKComplex = struct_rk_complex# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 233

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 241
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

RKIQZ = struct_rk_iqz# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 241

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 251
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

RKModuloPath = struct_rk_modulo_path# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 251

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 257
class struct_anon_66(Structure):
    pass

struct_anon_66._pack_ = 1
struct_anon_66.__slots__ = [
    'byte',
]
struct_anon_66._fields_ = [
    ('byte', RKByte * int(4)),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 258
class struct_anon_67(Structure):
    pass

struct_anon_67._pack_ = 1
struct_anon_67.__slots__ = [
    'u8',
    'u8_2',
    'u8_3',
    'u8_4',
]
struct_anon_67._fields_ = [
    ('u8', uint8_t),
    ('u8_2', uint8_t),
    ('u8_3', uint8_t),
    ('u8_4', uint8_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 259
class struct_anon_68(Structure):
    pass

struct_anon_68._pack_ = 1
struct_anon_68.__slots__ = [
    'i8',
    'i8_2',
    'i8_3',
    'i8_4',
]
struct_anon_68._fields_ = [
    ('i8', c_int8),
    ('i8_2', c_int8),
    ('i8_3', c_int8),
    ('i8_4', c_int8),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 260
class struct_anon_69(Structure):
    pass

struct_anon_69._pack_ = 1
struct_anon_69.__slots__ = [
    'u16',
    'u16_2',
]
struct_anon_69._fields_ = [
    ('u16', uint16_t),
    ('u16_2', uint16_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 261
class struct_anon_70(Structure):
    pass

struct_anon_70._pack_ = 1
struct_anon_70.__slots__ = [
    'i16',
    'i16_2',
]
struct_anon_70._fields_ = [
    ('i16', c_int16),
    ('i16_2', c_int16),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 262
class struct_anon_71(Structure):
    pass

struct_anon_71._pack_ = 1
struct_anon_71.__slots__ = [
    'u32',
]
struct_anon_71._fields_ = [
    ('u32', uint32_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 263
class struct_anon_72(Structure):
    pass

struct_anon_72._pack_ = 1
struct_anon_72.__slots__ = [
    'i32',
]
struct_anon_72._fields_ = [
    ('i32', c_int32),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 264
class struct_anon_73(Structure):
    pass

struct_anon_73._pack_ = 1
struct_anon_73.__slots__ = [
    'f',
]
struct_anon_73._fields_ = [
    ('f', c_float),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 265
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
    ('unnamed_1', struct_anon_66),
    ('unnamed_2', struct_anon_67),
    ('unnamed_3', struct_anon_68),
    ('unnamed_4', struct_anon_69),
    ('unnamed_5', struct_anon_70),
    ('unnamed_6', struct_anon_71),
    ('unnamed_7', struct_anon_72),
    ('unnamed_8', struct_anon_73),
]

RKFourByte = union_rk_four_byte# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 265

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 271
class struct_anon_74(Structure):
    pass

struct_anon_74._pack_ = 1
struct_anon_74.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_74._fields_ = [
    ('m', uint16_t, 10),
    ('e', c_int8, 5),
    ('s', uint8_t, 1),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 278
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
    ('unnamed_1', struct_anon_74),
    ('bytes', uint8_t * int(2)),
    ('word', uint16_t),
]

RKWordFloat16 = union_rk_half_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 278

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 284
class struct_anon_75(Structure):
    pass

struct_anon_75._pack_ = 1
struct_anon_75.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_75._fields_ = [
    ('m', uint32_t, 23),
    ('e', c_int8, 8),
    ('s', uint8_t, 1),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 292
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
    ('unnamed_1', struct_anon_75),
    ('bytes', uint8_t * int(4)),
    ('word', uint32_t),
    ('value', c_float),
]

RKWordFloat32 = union_rk_single_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 292

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 298
class struct_anon_76(Structure):
    pass

struct_anon_76._pack_ = 1
struct_anon_76.__slots__ = [
    'm',
    'e',
    's',
]
struct_anon_76._fields_ = [
    ('m', uint64_t, 52),
    ('e', c_int16, 11),
    ('s', uint8_t, 1),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 306
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
    ('unnamed_1', struct_anon_76),
    ('bytes', uint8_t * int(8)),
    ('word', uint64_t),
    ('value', c_double),
]

RKWordFloat64 = union_rk_double_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 306

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 309
class struct_anon_77(Structure):
    pass

struct_anon_77._pack_ = 1
struct_anon_77.__slots__ = [
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
struct_anon_77._fields_ = [
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 324
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
    ('unnamed_1', struct_anon_77),
    ('bytes', c_char * int(64)),
]

RKFilterAnchor = union_rk_filter_anchor# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 324

RKFilterAnchorGroup = RKFilterAnchor * int(8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 326

enum_anon_78 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultSuccess = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultTooBig = (RKResultSuccess + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultTimeout = (RKResultTooBig + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNullInput = (RKResultTimeout + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultEngineNotWired = (RKResultNullInput + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultEngineNotActive = (RKResultEngineNotWired + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteSend = (RKResultEngineNotActive + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteReceive = (RKResultIncompleteSend + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteTransceiver = (RKResultIncompleteReceive + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompletePedestal = (RKResultIncompleteTransceiver + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteHealthRelay = (RKResultIncompletePedestal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteControl = (RKResultIncompleteHealthRelay + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteWaveformCalibration = (RKResultIncompleteControl + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteProductDescription = (RKResultIncompleteWaveformCalibration + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultIncompleteScanDescription = (RKResultIncompleteProductDescription + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultErrorCreatingOperatorRoutine = (RKResultIncompleteScanDescription + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultErrorCreatingOperatorCommandRoutine = (RKResultErrorCreatingOperatorRoutine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultErrorCreatingClientRoutine = (RKResultErrorCreatingOperatorCommandRoutine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultSDToFDError = (RKResultErrorCreatingClientRoutine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoPulseBuffer = (RKResultSDToFDError + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoRayBuffer = (RKResultNoPulseBuffer + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoPulseCompressionEngine = (RKResultNoRayBuffer + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoPulseRingEngine = (RKResultNoPulseCompressionEngine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoMomentEngine = (RKResultNoPulseRingEngine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartCompressionCore = (RKResultNoMomentEngine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartRingCore = (RKResultFailedToStartCompressionCore + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPulseWatcher = (RKResultFailedToStartRingCore + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartRingPulseWatcher = (RKResultFailedToStartPulseWatcher + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToInitiateSemaphore = (RKResultFailedToStartRingPulseWatcher + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToRetrieveSemaphore = (RKResultFailedToInitiateSemaphore + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToAllocateFFTSpace = (RKResultFailedToRetrieveSemaphore + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToAllocateFilter = (RKResultFailedToAllocateFFTSpace + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToAllocateDutyCycleBuffer = (RKResultFailedToAllocateFilter + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToAllocateScratchSpace = (RKResultFailedToAllocateDutyCycleBuffer + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToSetWaveform = (RKResultFailedToAllocateScratchSpace + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToSetFilter = (RKResultFailedToSetWaveform + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultEngineDeactivatedMultipleTimes = (RKResultFailedToSetFilter + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartMomentCore = (RKResultEngineDeactivatedMultipleTimes + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPulseGatherer = (RKResultFailedToStartMomentCore + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultUnableToChangeCoreCounts = (RKResultFailedToStartPulseGatherer + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPedestalWorker = (RKResultUnableToChangeCoreCounts + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToGetVacantPosition = (RKResultFailedToStartPedestalWorker + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToGetVacantHealth = (RKResultFailedToGetVacantPosition + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartRayGatherer = (RKResultFailedToGetVacantHealth + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartHealthWorker = (RKResultFailedToStartRayGatherer + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPulseRecorder = (RKResultFailedToStartHealthWorker + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPedestalMonitor = (RKResultFailedToStartPulseRecorder + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartpedestalVcpEngine = (RKResultFailedToStartPedestalMonitor + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartFileManager = (RKResultFailedToStartpedestalVcpEngine + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartFileRemover = (RKResultFailedToStartFileManager + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartTransceiver = (RKResultFailedToStartFileRemover + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartPedestal = (RKResultFailedToStartTransceiver + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartHealthRelay = (RKResultFailedToStartPedestal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultPreferenceFileNotFound = (RKResultFailedToStartHealthRelay + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultPreferenceKeywordNotFound = (RKResultPreferenceFileNotFound + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToMeasureNoise = (RKResultPreferenceKeywordNotFound + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToEstimateNoise = (RKResultFailedToMeasureNoise + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToCreateFileRemover = (RKResultFailedToEstimateNoise + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFileManagerBufferNotResuable = (RKResultFailedToCreateFileRemover + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultInvalidMomentParameters = (RKResultFileManagerBufferNotResuable + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToCreateUnitWorker = (RKResultInvalidMomentParameters + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartHostWatcher = (RKResultFailedToCreateUnitWorker + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToStartHostPinger = (RKResultFailedToStartHostWatcher + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToExecuteCommand = (RKResultFailedToStartHostPinger + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToSetVCP = (RKResultFailedToExecuteCommand + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToAddHost = (RKResultFailedToSetVCP + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToFindProductId = (RKResultFailedToAddHost + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToOpenFileForProduct = (RKResultFailedToFindProductId + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultClientNotConnected = (RKResultFailedToOpenFileForProduct + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFileManagerInconsistentFolder = (RKResultClientNotConnected + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToExpandWaveform = (RKResultFileManagerInconsistentFolder + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultFailedToOpenFileForWriting = (RKResultFailedToExpandWaveform + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultRadarNotLive = (RKResultFailedToOpenFileForWriting + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultRawDataTypeUndefined = (RKResultRadarNotLive + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNothingToRead = (RKResultRawDataTypeUndefined + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultNoRadar = (RKResultNothingToRead + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

RKResultCount = (RKResultNoRadar + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 409

enum_RKEngineColor = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorCommandCenter = 14# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorRadarHubReporter = 10# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorPulseCompressionEngine = 7# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorPulseRingFilterEngine = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorPositionEngine = 4# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorSteerEngine = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorMomentEngine = 15# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorHealthEngine = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorDataRecorder = 12# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorSweepEngine = 18# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorHealthLogger = 5# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorFileManager = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorTransceiver = 17# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorPedestalRelayPedzy = 15# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorHealthRelayTweeta = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorHealthRelayNaveen = 11# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorRadarRelay = 17# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorHostMonitor = 16# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorClock = 19# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorMisc = 20# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorEngineMonitor = 19# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorConfig = 6# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorFFTModule = 19# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKEngineColorWebSocket = 8# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 415

RKValueType = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 442

enum_anon_79 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeBool = (RKValueTypeNull + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt = (RKValueTypeBool + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeLong = (RKValueTypeInt + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt8 = (RKValueTypeLong + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt16 = (RKValueTypeInt8 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt32 = (RKValueTypeInt16 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt64 = (RKValueTypeInt32 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeSSize = (RKValueTypeInt64 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt = (RKValueTypeSSize + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeULong = (RKValueTypeUInt + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt8 = (RKValueTypeULong + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt16 = (RKValueTypeUInt8 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt32 = (RKValueTypeUInt16 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt64 = (RKValueTypeUInt32 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeIntInHex = (RKValueTypeUInt64 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeLongInHex = (RKValueTypeIntInHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt8InHex = (RKValueTypeLongInHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt16InHex = (RKValueTypeInt8InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt32InHex = (RKValueTypeInt16InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeInt64InHex = (RKValueTypeInt32InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeSSizeInHex = (RKValueTypeInt64InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUIntInHex = (RKValueTypeSSizeInHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeULongInHex = (RKValueTypeUIntInHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt8InHex = (RKValueTypeULongInHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt16InHex = (RKValueTypeUInt8InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt32InHex = (RKValueTypeUInt16InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeUInt64InHex = (RKValueTypeUInt32InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeSize = (RKValueTypeUInt64InHex + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeFloat = (RKValueTypeSize + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeDouble = (RKValueTypeFloat + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeString = (RKValueTypeDouble + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeNumericString = (RKValueTypeString + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeFloatMultipliedBy1k = (RKValueTypeNumericString + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeFloatMultipliedBy1M = (RKValueTYpeFloatMultipliedBy1k + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeFloatDividedBy1k = (RKValueTYpeFloatMultipliedBy1M + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeFloatDividedBy1M = (RKValueTYpeFloatDividedBy1k + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeDoubleMultipliedBy1k = (RKValueTYpeFloatDividedBy1M + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeDoubleMultipliedBy1M = (RKValueTYpeDoubleMultipliedBy1k + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeDoubleDividedBy1k = (RKValueTYpeDoubleMultipliedBy1M + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTYpeDoubleDividedBy1M = (RKValueTYpeDoubleDividedBy1k + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeProductId = RKValueTypeInt8# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeIdentifier = RKValueTypeUInt64# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeDictionary = (RKValueTypeIdentifier + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeArray = (RKValueTypeDictionary + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKValueTypeVariable = (RKValueTypeArray + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 443

RKPositionFlag = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 492

enum_anon_80 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthEnabled = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthSafety = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthError = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthSweep = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthPoint = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthComplete = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationEnabled = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationSafety = (1 << 17)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationError = (1 << 18)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationSweep = (1 << 24)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationPoint = (1 << 25)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationComplete = (1 << 26)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagHardwareMask = 0x0FFFFFFF# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagScanActive = (1 << 28)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagVCPActive = (1 << 29)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagUsed = (1 << 30)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagReady = (1 << 31)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagAzimuthModeMask = (RKPositionFlagAzimuthSweep | RKPositionFlagAzimuthPoint)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagElevationModeMask = (RKPositionFlagElevationSweep | RKPositionFlagElevationPoint)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKPositionFlagScanModeMask = (RKPositionFlagAzimuthModeMask | RKPositionFlagElevationModeMask)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 493

RKHeadingType = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 517

enum_anon_81 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 518

RKHeadingTypeNormal = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 518

RKHeadingTypeAdd90 = (RKHeadingTypeNormal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 518

RKHeadingTypeAdd180 = (RKHeadingTypeAdd90 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 518

RKHeadingTypeAdd270 = (RKHeadingTypeAdd180 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 518

RKStatusFlag = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 525

enum_anon_82 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 526

RKStatusFlagVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 526

RKStatusFlagReady = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 526

RKHealthFlag = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 531

enum_anon_83 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 532

RKHealthFlagVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 532

RKHealthFlagReady = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 532

RKHealthFlagUsed = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 532

RKMarker = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 538

enum_anon_84 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerSweepMiddle = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerSweepBegin = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerSweepEnd = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerVolumeBegin = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerVolumeEnd = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerScanTypeMask = 0x60# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerScanTypeUnknown = (0 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerScanTypePPI = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerScanTypeRHI = (2 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerScanTytpePoint = (3 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKMarkerMemoryManagement = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 539

RKPulseStatus = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 569

enum_anon_85 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusHasIQData = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusHasPosition = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusInspected = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusCompressed = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusSkipped = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusDownSampled = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusProcessed = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusRingInspected = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusRingFiltered = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusRingSkipped = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusRingProcessed = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusReadyForMoments = ((((RKPulseStatusProcessed | RKPulseStatusCompressed) | RKPulseStatusRingProcessed) | RKPulseStatusHasPosition) | RKPulseStatusHasIQData)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusUsedForMoments = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusProcessMask = (((((((((RKPulseStatusInspected | RKPulseStatusCompressed) | RKPulseStatusSkipped) | RKPulseStatusDownSampled) | RKPulseStatusProcessed) | RKPulseStatusRingInspected) | RKPulseStatusRingFiltered) | RKPulseStatusRingSkipped) | RKPulseStatusRingProcessed) | RKPulseStatusUsedForMoments)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusRecorded = (1 << 12)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusStreamed = (1 << 13)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKPulseStatusConsumed = (1 << 14)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 570

RKRayStatus = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 605

enum_anon_86 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusProcessing = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusProcessed = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusSkipped = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusReady = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusStreamed = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusConsumed = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKRayStatusOverviewed = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 606

RKInitFlag = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 617

enum_anon_87 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagNone = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagVerbose = 0x00000001# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagVeryVerbose = 0x00000002# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagVeryVeryVerbose = 0x00000004# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagShowClockOffset = 0x00000008# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagManuallyAssignCPU = 0x00000010# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagIgnoreGPS = 0x00000020# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagIgnoreHeading = 0x00000040# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagReserved4 = 0x00000080# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocStatusBuffer = 0x00000100# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocConfigBuffer = 0x00000200# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocRawIQBuffer = 0x00000400# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocPositionBuffer = 0x00000800# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocMomentBuffer = 0x00001000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocHealthBuffer = 0x00002000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocHealthNodes = 0x00004000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagReserved1 = 0x00008000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagPulsePositionCombiner = 0x00010000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagPositionSteerEngine = 0x00020000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagSignalProcessor = 0x00040000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagStartPulseEngine = 0x00100000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagStartRingFilterEngine = 0x00200000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagStartMomentEngine = 0x00400000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagRelay = 0x00007703# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagIQPlayback = 0x00047701# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocEverything = 0x00077F01# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKInitFlagAllocEverythingQuiet = 0x00077F00# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 618

RKMomentList = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 650

enum_anon_88 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHm = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHmi = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHmq = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR0 = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR1 = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR1i = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR1q = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR2 = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR3 = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListHR4 = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVm = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVmi = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVmq = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR0 = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR1 = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR1i = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR1q = (1 << 12)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR2 = (1 << 13)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR3 = (1 << 14)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListVR4 = (1 << 15)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListC0 = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListC0i = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListC0q = (1 << 17)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCn1 = (1 << 18)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCp1 = (1 << 19)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCn2 = (1 << 20)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCp2 = (1 << 21)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCn3 = (1 << 22)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCp3 = (1 << 23)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCn4 = (1 << 24)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCp4 = (1 << 25)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCa0 = (1 << 26)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCb0 = (1 << 27)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListChcvx0 = (1 << 28)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentListCvchx0 = (1 << 29)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 651

RKMomentIndex = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 690

enum_anon_89 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHmi = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHmq = (RKMomentIndexHmi + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR0 = (RKMomentIndexHmq + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR1i = (RKMomentIndexHR0 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR1q = (RKMomentIndexHR1i + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR2 = (RKMomentIndexHR1q + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR3 = (RKMomentIndexHR2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexHR4 = (RKMomentIndexHR3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVmi = (RKMomentIndexHR4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVmq = (RKMomentIndexVmi + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR0 = (RKMomentIndexVmq + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR1i = (RKMomentIndexVR0 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR1q = (RKMomentIndexVR1i + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR2 = (RKMomentIndexVR1q + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR3 = (RKMomentIndexVR2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexVR4 = (RKMomentIndexVR3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexC0i = (RKMomentIndexVR4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexC0q = (RKmomentIndexC0i + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCn1 = (RKmomentIndexC0q + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCp1 = (RKmomentIndexCn1 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCn2 = (RKmomentIndexCp1 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCp2 = (RKmomentIndexCn2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCn3 = (RKmomentIndexCp2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCp3 = (RKmomentIndexCn3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCn4 = (RKmomentIndexCp3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKmomentIndexCp4 = (RKmomentIndexCn4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKMomentIndexCount = (RKmomentIndexCp4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 691

RKBaseProductList = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 722

enum_anon_90 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListNone = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8Z = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8V = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8W = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8D = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8P = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8R = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8K = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8Sh = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8Sv = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8Q = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U6 = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U5 = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U4 = (1 << 12)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U3 = (1 << 13)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U2 = (1 << 14)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8U1 = (1 << 15)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8ZVWDPR = 0x0000003F# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8ZVWDPRK = 0x0000007F# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8ZVWDPRKS = 0x000001FF# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8ZVWDPRKSQ = 0x000003FF# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListUInt8All = 0x0000FFFF# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZ = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatV = (1 << 17)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatW = (1 << 18)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatD = (1 << 19)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatP = (1 << 20)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatR = (1 << 21)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatK = (1 << 22)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatSh = (1 << 23)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatSv = (1 << 24)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatQ = (1 << 25)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatLh = (1 << 26)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatLv = (1 << 27)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatRXh = (1 << 28)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatRXv = (1 << 29)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatPXh = (1 << 30)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatPXv = (1 << 31)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZVWDPR = 0x003F0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZVWDPRK = 0x007F0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZVWDPRKS = 0x01FF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZVWDPRKLRXPX = 0xFC7F0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatATSR = 0xFDFF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatZVWDPRKSQ = 0x03FF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductListFloatAll = 0xFFFF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 723

RKBaseProductIndex = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 771

enum_anon_91 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexZ = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexV = (RKBaseProductIndexZ + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexW = (RKBaseProductIndexV + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexD = (RKBaseProductIndexW + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexP = (RKBaseProductIndexD + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexR = (RKBaseProductIndexP + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexK = (RKBaseProductIndexR + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexSh = (RKBaseProductIndexK + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexSv = (RKBaseProductIndexSh + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexQ = (RKBaseProductIndexSv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexLh = (RKBaseProductIndexQ + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexLv = (RKBaseProductIndexLh + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexRXh = (RKBaseProductIndexLv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexRXv = (RKBaseProductIndexRXh + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexPXh = (RKBaseProductIndexRXv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexPXv = (RKBaseProductIndexPXh + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexZv = (RKBaseProductIndexPXv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexVv = (RKBaseProductIndexZv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexWv = (RKBaseProductIndexVv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKBaseProductIndexCount = (RKBaseProductIndexWv + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 772

RKProductType = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 795

enum_anon_92 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 796

RKProductTypeUnknown = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 796

RKProductTypeCellMatch = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 796

RKProductTypePPI = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 796

RKProductTypeCAPPI = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 796

RKConfigKey = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 803

enum_anon_93 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySweepElevation = (RKConfigKeyNull + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySweepAzimuth = (RKConfigKeySweepElevation + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPositionMarker = (RKConfigKeySweepAzimuth + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPRT = (RKConfigKeyPositionMarker + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPRF = (RKConfigKeyPRT + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyDualPRF = (RKConfigKeyPRF + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPulseGateCount = (RKConfigKeyDualPRF + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPulseGateSize = (RKConfigKeyPulseGateCount + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyPulseWidth = (RKConfigKeyPulseGateSize + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyWaveform = (RKConfigKeyPulseWidth + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyWaveformDecimate = (RKConfigKeyWaveform + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyWaveformId = (RKConfigKeyWaveformDecimate + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyWaveformName = (RKConfigKeyWaveformId + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySystemNoise = (RKConfigKeyWaveformName + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySystemZCal = (RKConfigKeySystemNoise + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySystemDCal = (RKConfigKeySystemZCal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySystemPCal = (RKConfigKeySystemDCal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyWaveformCalibration = (RKConfigKeySystemPCal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySNRThreshold = (RKConfigKeyWaveformCalibration + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeySQIThreshold = (RKConfigKeySNRThreshold + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyVCPDefinition = (RKConfigKeySQIThreshold + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyRingFilterGateCount = (RKConfigKeyVCPDefinition + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyTransitionGateCount = (RKConfigKeyRingFilterGateCount + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyUserIntegerParameters = (RKConfigKeyTransitionGateCount + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyUserFloatParameters = (RKConfigKeyUserIntegerParameters + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyUserResource = (RKConfigKeyUserFloatParameters + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKConfigKeyCount = (RKConfigKeyUserResource + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 804

RKHealthNode = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 835

enum_anon_94 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeRadarKit = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeTransceiver = (RKHealthNodeRadarKit + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodePedestal = (RKHealthNodeTransceiver + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeTweeta = (RKHealthNodePedestal + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser1 = (RKHealthNodeTweeta + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser2 = (RKHealthNodeUser1 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser3 = (RKHealthNodeUser2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser4 = (RKHealthNodeUser3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser5 = (RKHealthNodeUser4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser6 = (RKHealthNodeUser5 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser7 = (RKHealthNodeUser6 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeUser8 = (RKHealthNodeUser7 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeCount = (RKHealthNodeUser8 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKHealthNodeInvalid = (RKHealthNode (ord_if_char((-1)))).value# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 836

RKScriptProperty = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 853

enum_anon_95 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyProduceZip = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyProduceTgz = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyProduceTarXz = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyProduceTxz = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyProduceArchive = (((RKScriptPropertyProduceZip | RKScriptPropertyProduceTgz) | RKScriptPropertyProduceTarXz) | RKScriptPropertyProduceTxz)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKScriptPropertyRemoveNCFiles = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 854

RKEngineState = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 890

enum_anon_96 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSleep0 = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSleep1 = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSleep2 = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSleep3 = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSleepMask = 0x0000000F# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateWritingFile = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateMemoryChange = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateSuspended = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateBusyMask = 0x000000F0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateReserved = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateAllocated = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateProperlyWired = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateActivating = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateDeactivating = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateActive = (1 << 13)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateWantActive = (1 << 15)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateMainMask = 0x0000FF00# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildAllocated = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildProperlyWired = (1 << 17)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildActivating = (1 << 18)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildDeactivating = (1 << 19)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildActive = (1 << 20)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKEngineStateChildMask = 0x001F0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 891

RKStatusEnum = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 918

enum_anon_97 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumUnknown = (-3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumOld = (-3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumInvalid = (-2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumTooLow = (-2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumLow = (-1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumNormal = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumActive = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumHigh = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumStandby = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumInactive = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumOutOfRange = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumTooHigh = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumNotOperational = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumOff = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumFault = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumNotWired = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKStatusEnumCritical = 4# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 919

RKFileType = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 939

enum_anon_98 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKFileTypeIQ = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKFileTypeMoment = (RKFileTypeIQ + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKFileTypeHealth = (RKFileTypeMoment + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKFileTypeLog = (RKFileTypeHealth + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKFileTypeCount = (RKFileTypeLog + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 940

RKStream = uint64_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 948

enum_anon_99 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusMask = 0x0F# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusPositions = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusPulses = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusRays = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusIngest = 4# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusEngines = 5# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusBuffers = 6# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamASCIIArtZ = 7# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamASCIIArtHealth = 8# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamASCIIArtVCP = 9# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusAll = 0xFF# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamHealthInJSON = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusEngineBinary = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusProcessorStatus = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayIQ = (1 << 8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayIQFiltered = (1 << 9)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductIQ = (1 << 10)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductIQFiltered = (1 << 11)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamControl = (1 << 15)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamScopeStuff = 0x0000000000000300# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayZ = (1 << 16)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayV = (1 << 17)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayW = (1 << 18)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayD = (1 << 19)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayP = (1 << 20)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayR = (1 << 21)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayK = (1 << 22)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplaySh = (1 << 23)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplaySv = (1 << 24)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayQ = (1 << 25)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayZVWDPRKS = 0x0000000001FF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamDisplayAll = 0x0000000003FF0000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductZ = (1 << 32)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductV = (1 << 33)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductW = (1 << 34)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductD = (1 << 35)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductP = (1 << 36)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductR = (1 << 37)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductK = (1 << 38)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductSh = (1 << 39)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductSv = (1 << 40)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductQ = (1 << 41)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductZVWDPRKS = 0x000001FF00000000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamProductAll = 0x000003FF00000000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepZ = (1 << 48)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepV = (1 << 49)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepW = (1 << 50)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepD = (1 << 51)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepP = (1 << 52)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepR = (1 << 53)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepK = (1 << 54)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepSh = (1 << 55)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepSv = (1 << 56)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepQ = (1 << 57)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepZVWDPRKS = 0x01FF000000000000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamSweepAll = 0x03FF000000000000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamAlmostEverything = 0x03FF03FF03FFF000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKStreamStatusTerminalChange = 0x0400000000000000# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 949

RKHostStatus = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1011

enum_anon_100 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKHostStatusUnknown = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKHostStatusUnreachable = (RKHostStatusUnknown + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKHostStatusPartiallyReachable = (RKHostStatusUnreachable + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKHostStatusReachableUnusual = (RKHostStatusPartiallyReachable + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKHostStatusReachable = (RKHostStatusReachableUnusual + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1012

RKProductStatus = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1020

enum_anon_101 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusVacant = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusActive = (1 << 0)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusBusy = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusSkipped = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusSleep0 = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusSleep1 = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusSleep2 = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKProductStatusSleep3 = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1021

RKTextPreferences = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1032

enum_anon_102 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesNone = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesShowColor = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesDrawBackground = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSizeMask = (7 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize80x25 = (0 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize80x40 = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize80x50 = (2 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize120x40 = (3 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize120x50 = (4 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesWindowSize120x80 = (5 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKTextPreferencesShowDebuggingMessage = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1033

RKWaveformType = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1047

enum_anon_103 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeNone = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeIsComplex = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeSingleTone = (1 << 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeFrequencyHopping = (1 << 2)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeLinearFrequencyModulation = (1 << 3)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeTimeFrequencyMultiplexing = (1 << 4)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeFromFile = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeFlatAnchors = (1 << 6)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKWaveformTypeFrequencyHoppingChirp = (1 << 7)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1048

RKEventType = uint32_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1060

enum_anon_104 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1061

RKEventTypeNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1061

RKEventTypeRaySweepBegin = (RKEventTypeNull + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1061

RKEventTypeRaySweepEnd = (RKEventTypeRaySweepBegin + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1061

RKFilterType = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1067

enum_anon_105 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeElliptical1 = (RKFilterTypeNull + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeElliptical2 = (RKFilterTypeElliptical1 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeElliptical3 = (RKFilterTypeElliptical2 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeElliptical4 = (RKFilterTypeElliptical3 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeImpulse = (RKFilterTypeElliptical4 + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeCount = (RKFilterTypeImpulse + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeUserDefined = (RKFilterTypeCount + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKFilterTypeTest1 = (RKFilterTypeUserDefined + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1068

RKPedestalInstructType = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1080

enum_anon_106 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeNone = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeStandby = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeEnable = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeDisable = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeSlew = 4# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModePoint = 5# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeReset = 6# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeTest = 7# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeModeMask = 0x0F# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeAxisElevation = 0x10# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeAxisAzimuth = 0x20# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKPedestalInstructTypeAxisMask = 0x30# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1081

RKRawDataType = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1096

enum_anon_107 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1097

RKRawDataTypeNull = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1097

RKRawDataTypeFromTransceiver = (RKRawDataTypeNull + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1097

RKRawDataTypeAfterMatchedFilter = (RKRawDataTypeFromTransceiver + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1097

RKCompressorOption = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1103

enum_anon_108 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1104

RKCompressorOptionRKInt16C = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1104

RKCompressorOptionRKComplex = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1104

RKCompressorOptionSingleChannel = (1 << 5)# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1104

RadarHubType = uint8_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1110

enum_anon_109 = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeHandshake = 1# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeControl = 2# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeHealth = 3# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserve4 = 4# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeScope = 5# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeResponse = 6# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved7 = 7# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved8 = 8# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved9 = 9# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved10 = 10# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved11 = 11# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved12 = 12# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved13 = 13# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved14 = 14# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeReserved15 = 15# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialZ = 16# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialV = 17# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialW = 18# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialD = 19# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialP = 20# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

RKRadarHubTypeRadialR = 21# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1111

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1136
class struct_anon_110(Structure):
    pass

struct_anon_110._pack_ = 1
struct_anon_110.__slots__ = [
    'type',
    'startElevation',
    'endElevation',
    'startAzimuth',
    'endAzimuth',
]
struct_anon_110._fields_ = [
    ('type', uint8_t),
    ('startElevation', c_int16),
    ('endElevation', c_int16),
    ('startAzimuth', c_int16),
    ('endAzimuth', c_int16),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1144
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
    ('bytes', RKByte * int(10)),
]

RKRadarHubRayHeader = union_rk_radarhub_ray_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1144

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1147
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1152
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

RKRadarHubRay = union_rk_radarhub_ray# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1152

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1197
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

RKRadarDesc = struct_rk_radar_desc# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1197

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1210
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
    ('depth', c_int),
    ('fc', c_double),
    ('fs', c_double),
    ('filterCounts', uint8_t * int(22)),
    ('filterAnchors', RKFilterAnchorGroup * int(22)),
    ('samples', POINTER(RKComplex) * int(22)),
    ('iSamples', POINTER(RKInt16C) * int(22)),
]

RKWaveform = struct_rk_waveform# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1210

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1213
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1223
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

RKWaveFileGlobalHeader = union_rk_wave_file_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1223

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1232
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

RKWaveformCalibration = struct_rk_waveform_cal# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1232

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1239
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

RKWaveformResponse = struct_rk_waveform_response# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1239

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1245
class struct_anon_113(Structure):
    pass

struct_anon_113._pack_ = 1
struct_anon_113.__slots__ = [
    'i',
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
    'userIntegerParameters',
    'userFloatParameters',
    'vcpDefinition',
]
struct_anon_113._fields_ = [
    ('i', RKIdentifier),
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
    ('userIntegerParameters', uint32_t * int(8)),
    ('userFloatParameters', c_float * int(8)),
    ('vcpDefinition', c_char * int(512)),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1275
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

RKConfig = union_rk_config# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1275

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1281
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1289
class union_rk_heath(Union):
    pass

union_rk_heath._pack_ = 1
union_rk_heath.__slots__ = [
    'unnamed_1',
    'bytes',
]
union_rk_heath._anonymous_ = [
    'unnamed_1',
]
union_rk_heath._fields_ = [
    ('unnamed_1', struct_anon_114),
    ('bytes', POINTER(RKByte)),
]

RKHealth = union_rk_heath# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1289

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1298
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

RKNodalHealth = struct_rk_nodal_health# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1298

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1304
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
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1332
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

RKPosition = union_rk_position# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1332

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1339
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

RKScanAction = struct_rk_scan_action# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1339

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1342
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1367
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

RKPulseHeader = union_rk_pulse_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1367

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1377
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

RKPulseParameters = struct_rk_pulse_parameters# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1377

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1386
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1385
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1393
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

RKPulse = struct_rk_pulse# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1393

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1425
class struct_rk_ray_header(Structure):
    pass

struct_rk_ray_header._pack_ = 1
struct_rk_ray_header.__slots__ = [
    'capacity',
    's',
    'i',
    'n',
    'marker',
    'baseMomentList',
    'baseProductList',
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
    ('baseMomentList', RKMomentList),
    ('baseProductList', RKBaseProductList),
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

RKRayHeader = struct_rk_ray_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1425

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1433
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1438
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

RKRay = struct_rk_ray# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1438

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1458
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
    'baseProductList',
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
    ('startTime', time_t),
    ('endTime', time_t),
    ('momentList', RKMomentList),
    ('baseProductList', RKBaseProductList),
    ('gateSizeMeters', c_float),
    ('isPPI', c_bool),
    ('isRHI', c_bool),
    ('external', c_bool),
    ('desc', RKRadarDesc),
    ('config', RKConfig),
    ('filename', c_char * int((1024 - 80))),
]

RKSweepHeader = struct_rk_sweep_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1458

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1467
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

RKSweep = struct_rk_sweep# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1467

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1470
class struct_anon_120(Structure):
    pass

struct_anon_120._pack_ = 1
struct_anon_120.__slots__ = [
    'preface',
    'version',
    'dataType',
    'reserved',
    'desc',
    'config',
]
struct_anon_120._fields_ = [
    ('preface', RKName),
    ('version', uint32_t),
    ('dataType', RKRawDataType),
    ('reserved', uint8_t * int(123)),
    ('desc', RKRadarDesc),
    ('config', RKConfig),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1479
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

RKFileHeader = union_rk_file_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1479

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1493
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

RKPreferenceObject = struct_rk_preferene_object# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1493

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1504
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

RKControl = struct_rk_control# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1504

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1528
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

RKStatus = struct_rk_status# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1528

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1542
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

RKSimpleEngine = struct_rk_simple_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1542

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1556
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

RKFileMonitor = struct_rk_file_monitor# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1556

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1563
class struct_anon_121(Structure):
    pass

struct_anon_121._pack_ = 1
struct_anon_121.__slots__ = [
    'key',
    'name',
    'unit',
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
    'baseProductList',
]
struct_anon_121._fields_ = [
    ('key', uint32_t),
    ('name', RKName),
    ('unit', RKName),
    ('colormap', RKName),
    ('symbol', c_char * int(8)),
    ('index', RKBaseProductIndex),
    ('type', RKProductType),
    ('pieceCount', uint32_t),
    ('w', RKFloat * int(16)),
    ('b', RKFloat * int(16)),
    ('l', RKFloat * int(16)),
    ('mininimumValue', RKFloat),
    ('maximumValue', RKFloat),
    ('baseProductList', RKBaseProductList),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1580
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

RKProductDesc = union_rk_product_desc# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1580

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1583
class struct_anon_122(Structure):
    pass

struct_anon_122._pack_ = 1
struct_anon_122.__slots__ = [
    'radarName',
    'latitude',
    'longitude',
    'heading',
    'radarHeight',
    'wavelength',
    'sweepElevation',
    'sweepAzimuth',
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
    'vcpDefinition',
    'suggestedFilename',
]
struct_anon_122._fields_ = [
    ('radarName', RKName),
    ('latitude', c_double),
    ('longitude', c_double),
    ('heading', c_float),
    ('radarHeight', c_float),
    ('wavelength', c_float),
    ('sweepElevation', c_float),
    ('sweepAzimuth', c_float),
    ('rayCount', uint32_t),
    ('gateCount', uint32_t),
    ('gateSizeMeters', c_float),
    ('startTime', time_t),
    ('endTime', time_t),
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
    ('vcpDefinition', c_char * int(512)),
    ('suggestedFilename', c_char * int(1024)),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1615
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

RKProductHeader = union_rk_product_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1615

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1630
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
    ('data', POINTER(RKFloat)),
]

RKProduct = struct_rk_product# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1630

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1635
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

RKProductCollection = struct_rk_product_collection# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1635

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1644
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

RKIIRFilter = struct_rk_iir_filter# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1644

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1650
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

RKTask = struct_rk_task# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1650

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1662
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

RKCommandQueue = struct_rk_command_queue# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1662

sa_family_t = c_ushort# /usr/include/x86_64-linux-gnu/bits/sockaddr.h: 28

# /usr/include/x86_64-linux-gnu/bits/socket.h: 180
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

enum_RKJSONObjectType = c_int# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

RKJSONObjectTypeUnknown = 0# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

RKJSONObjectTypePlain = (RKJSONObjectTypeUnknown + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

RKJSONObjectTypeString = (RKJSONObjectTypePlain + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

RKJSONObjectTypeArray = (RKJSONObjectTypeString + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

RKJSONObjectTypeObject = (RKJSONObjectTypeArray + 1)# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 109

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 122
if _libs["radarkit"].has("RKGetColor", "cdecl"):
    RKGetColor = _libs["radarkit"].get("RKGetColor", "cdecl")
    RKGetColor.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetColor.restype = ReturnString
    else:
        RKGetColor.restype = String
        RKGetColor.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 125
if _libs["radarkit"].has("RKGetColorOfIndex", "cdecl"):
    RKGetColorOfIndex = _libs["radarkit"].get("RKGetColorOfIndex", "cdecl")
    RKGetColorOfIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetColorOfIndex.restype = ReturnString
    else:
        RKGetColorOfIndex.restype = String
        RKGetColorOfIndex.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 128
if _libs["radarkit"].has("RKGetBackgroundColor", "cdecl"):
    RKGetBackgroundColor = _libs["radarkit"].get("RKGetBackgroundColor", "cdecl")
    RKGetBackgroundColor.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColor.restype = ReturnString
    else:
        RKGetBackgroundColor.restype = String
        RKGetBackgroundColor.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 131
if _libs["radarkit"].has("RKGetBackgroundColorOfIndex", "cdecl"):
    RKGetBackgroundColorOfIndex = _libs["radarkit"].get("RKGetBackgroundColorOfIndex", "cdecl")
    RKGetBackgroundColorOfIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColorOfIndex.restype = ReturnString
    else:
        RKGetBackgroundColorOfIndex.restype = String
        RKGetBackgroundColorOfIndex.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 134
if _libs["radarkit"].has("RKGetBackgroundColorOfCubeIndex", "cdecl"):
    RKGetBackgroundColorOfCubeIndex = _libs["radarkit"].get("RKGetBackgroundColorOfCubeIndex", "cdecl")
    RKGetBackgroundColorOfCubeIndex.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetBackgroundColorOfCubeIndex.restype = ReturnString
    else:
        RKGetBackgroundColorOfCubeIndex.restype = String
        RKGetBackgroundColorOfCubeIndex.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 144
if _libs["radarkit"].has("RKExtractJSON", "cdecl"):
    RKExtractJSON = _libs["radarkit"].get("RKExtractJSON", "cdecl")
    RKExtractJSON.argtypes = [String, POINTER(uint8_t), String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKExtractJSON.restype = ReturnString
    else:
        RKExtractJSON.restype = String
        RKExtractJSON.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 147
if _libs["radarkit"].has("RKGetValueOfKey", "cdecl"):
    RKGetValueOfKey = _libs["radarkit"].get("RKGetValueOfKey", "cdecl")
    RKGetValueOfKey.argtypes = [String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetValueOfKey.restype = ReturnString
    else:
        RKGetValueOfKey.restype = String
        RKGetValueOfKey.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 150
if _libs["radarkit"].has("RKReplaceAllValuesOfKey", "cdecl"):
    RKReplaceAllValuesOfKey = _libs["radarkit"].get("RKReplaceAllValuesOfKey", "cdecl")
    RKReplaceAllValuesOfKey.argtypes = [String, String, c_int]
    RKReplaceAllValuesOfKey.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 153
if _libs["radarkit"].has("RKReplaceEnumOfKey", "cdecl"):
    RKReplaceEnumOfKey = _libs["radarkit"].get("RKReplaceEnumOfKey", "cdecl")
    RKReplaceEnumOfKey.argtypes = [String, String, c_int]
    RKReplaceEnumOfKey.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 156
if _libs["radarkit"].has("RKReviseLogicalValues", "cdecl"):
    RKReviseLogicalValues = _libs["radarkit"].get("RKReviseLogicalValues", "cdecl")
    RKReviseLogicalValues.argtypes = [String]
    RKReviseLogicalValues.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 168
if _libs["radarkit"].has("RKJSONSkipWhiteSpaces", "cdecl"):
    RKJSONSkipWhiteSpaces = _libs["radarkit"].get("RKJSONSkipWhiteSpaces", "cdecl")
    RKJSONSkipWhiteSpaces.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONSkipWhiteSpaces.restype = ReturnString
    else:
        RKJSONSkipWhiteSpaces.restype = String
        RKJSONSkipWhiteSpaces.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 174
if _libs["radarkit"].has("RKJSONScanPassed", "cdecl"):
    RKJSONScanPassed = _libs["radarkit"].get("RKJSONScanPassed", "cdecl")
    RKJSONScanPassed.argtypes = [String, String, c_char]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONScanPassed.restype = ReturnString
    else:
        RKJSONScanPassed.restype = String
        RKJSONScanPassed.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 177
if _libs["radarkit"].has("RKJSONGetElement", "cdecl"):
    RKJSONGetElement = _libs["radarkit"].get("RKJSONGetElement", "cdecl")
    RKJSONGetElement.argtypes = [String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONGetElement.restype = ReturnString
    else:
        RKJSONGetElement.restype = String
        RKJSONGetElement.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 181
if _libs["radarkit"].has("RKJSONKeyValueFromElement", "cdecl"):
    RKJSONKeyValueFromElement = _libs["radarkit"].get("RKJSONKeyValueFromElement", "cdecl")
    RKJSONKeyValueFromElement.argtypes = [String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONKeyValueFromElement.restype = ReturnString
    else:
        RKJSONKeyValueFromElement.restype = String
        RKJSONKeyValueFromElement.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 190
if _libs["radarkit"].has("RKJSONGetValueOfKey", "cdecl"):
    RKJSONGetValueOfKey = _libs["radarkit"].get("RKJSONGetValueOfKey", "cdecl")
    RKJSONGetValueOfKey.argtypes = [String, String, String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKJSONGetValueOfKey.restype = ReturnString
    else:
        RKJSONGetValueOfKey.restype = String
        RKJSONGetValueOfKey.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 197
if _libs["radarkit"].has("RKUIntegerToCommaStyleString", "cdecl"):
    RKUIntegerToCommaStyleString = _libs["radarkit"].get("RKUIntegerToCommaStyleString", "cdecl")
    RKUIntegerToCommaStyleString.argtypes = [c_ulonglong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKUIntegerToCommaStyleString.restype = ReturnString
    else:
        RKUIntegerToCommaStyleString.restype = String
        RKUIntegerToCommaStyleString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 200
if _libs["radarkit"].has("RKIntegerToCommaStyleString", "cdecl"):
    RKIntegerToCommaStyleString = _libs["radarkit"].get("RKIntegerToCommaStyleString", "cdecl")
    RKIntegerToCommaStyleString.argtypes = [c_longlong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKIntegerToCommaStyleString.restype = ReturnString
    else:
        RKIntegerToCommaStyleString.restype = String
        RKIntegerToCommaStyleString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 203
if _libs["radarkit"].has("RKIntegerToHexStyleString", "cdecl"):
    RKIntegerToHexStyleString = _libs["radarkit"].get("RKIntegerToHexStyleString", "cdecl")
    RKIntegerToHexStyleString.argtypes = [c_longlong]
    if sizeof(c_int) == sizeof(c_void_p):
        RKIntegerToHexStyleString.restype = ReturnString
    else:
        RKIntegerToHexStyleString.restype = String
        RKIntegerToHexStyleString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 206
if _libs["radarkit"].has("RKFloatToCommaStyleString", "cdecl"):
    RKFloatToCommaStyleString = _libs["radarkit"].get("RKFloatToCommaStyleString", "cdecl")
    RKFloatToCommaStyleString.argtypes = [c_double]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFloatToCommaStyleString.restype = ReturnString
    else:
        RKFloatToCommaStyleString.restype = String
        RKFloatToCommaStyleString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 213
if _libs["radarkit"].has("RKNow", "cdecl"):
    RKNow = _libs["radarkit"].get("RKNow", "cdecl")
    RKNow.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKNow.restype = ReturnString
    else:
        RKNow.restype = String
        RKNow.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 216
if _libs["radarkit"].has("RKTimevalDiff", "cdecl"):
    RKTimevalDiff = _libs["radarkit"].get("RKTimevalDiff", "cdecl")
    RKTimevalDiff.argtypes = [struct_timeval, struct_timeval]
    RKTimevalDiff.restype = c_double

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 219
if _libs["radarkit"].has("RKTimespecDiff", "cdecl"):
    RKTimespecDiff = _libs["radarkit"].get("RKTimespecDiff", "cdecl")
    RKTimespecDiff.argtypes = [struct_timespec, struct_timespec]
    RKTimespecDiff.restype = c_double

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 222
if _libs["radarkit"].has("RKUTCTime", "cdecl"):
    RKUTCTime = _libs["radarkit"].get("RKUTCTime", "cdecl")
    RKUTCTime.argtypes = [POINTER(struct_timespec)]
    RKUTCTime.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 228
if _libs["radarkit"].has("RKFilenameExists", "cdecl"):
    RKFilenameExists = _libs["radarkit"].get("RKFilenameExists", "cdecl")
    RKFilenameExists.argtypes = [String]
    RKFilenameExists.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 229
if _libs["radarkit"].has("RKPreparePath", "cdecl"):
    RKPreparePath = _libs["radarkit"].get("RKPreparePath", "cdecl")
    RKPreparePath.argtypes = [String]
    RKPreparePath.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 230
if _libs["radarkit"].has("RKCountFilesInPath", "cdecl"):
    RKCountFilesInPath = _libs["radarkit"].get("RKCountFilesInPath", "cdecl")
    RKCountFilesInPath.argtypes = [String]
    RKCountFilesInPath.restype = c_long

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 231
if _libs["radarkit"].has("RKFolderOfFilename", "cdecl"):
    RKFolderOfFilename = _libs["radarkit"].get("RKFolderOfFilename", "cdecl")
    RKFolderOfFilename.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFolderOfFilename.restype = ReturnString
    else:
        RKFolderOfFilename.restype = String
        RKFolderOfFilename.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 232
if _libs["radarkit"].has("RKFileExtension", "cdecl"):
    RKFileExtension = _libs["radarkit"].get("RKFileExtension", "cdecl")
    RKFileExtension.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKFileExtension.restype = ReturnString
    else:
        RKFileExtension.restype = String
        RKFileExtension.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 233
if _libs["radarkit"].has("RKLastPartOfPath", "cdecl"):
    RKLastPartOfPath = _libs["radarkit"].get("RKLastPartOfPath", "cdecl")
    RKLastPartOfPath.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastPartOfPath.restype = ReturnString
    else:
        RKLastPartOfPath.restype = String
        RKLastPartOfPath.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 234
if _libs["radarkit"].has("RKLastTwoPartsOfPath", "cdecl"):
    RKLastTwoPartsOfPath = _libs["radarkit"].get("RKLastTwoPartsOfPath", "cdecl")
    RKLastTwoPartsOfPath.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastTwoPartsOfPath.restype = ReturnString
    else:
        RKLastTwoPartsOfPath.restype = String
        RKLastTwoPartsOfPath.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 235
if _libs["radarkit"].has("RKLastNPartsOfPath", "cdecl"):
    RKLastNPartsOfPath = _libs["radarkit"].get("RKLastNPartsOfPath", "cdecl")
    RKLastNPartsOfPath.argtypes = [String, c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastNPartsOfPath.restype = ReturnString
    else:
        RKLastNPartsOfPath.restype = String
        RKLastNPartsOfPath.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 236
if _libs["radarkit"].has("RKPathStringByExpandingTilde", "cdecl"):
    RKPathStringByExpandingTilde = _libs["radarkit"].get("RKPathStringByExpandingTilde", "cdecl")
    RKPathStringByExpandingTilde.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPathStringByExpandingTilde.restype = ReturnString
    else:
        RKPathStringByExpandingTilde.restype = String
        RKPathStringByExpandingTilde.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 237
if _libs["radarkit"].has("RKReplaceFileExtension", "cdecl"):
    RKReplaceFileExtension = _libs["radarkit"].get("RKReplaceFileExtension", "cdecl")
    RKReplaceFileExtension.argtypes = [String, String, String]
    RKReplaceFileExtension.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 243
if _libs["radarkit"].has("RKSignalString", "cdecl"):
    RKSignalString = _libs["radarkit"].get("RKSignalString", "cdecl")
    RKSignalString.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSignalString.restype = ReturnString
    else:
        RKSignalString.restype = String
        RKSignalString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 249
if _libs["radarkit"].has("RKStripTail", "cdecl"):
    RKStripTail = _libs["radarkit"].get("RKStripTail", "cdecl")
    RKStripTail.argtypes = [String]
    RKStripTail.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 250
if _libs["radarkit"].has("RKUnquote", "cdecl"):
    RKUnquote = _libs["radarkit"].get("RKUnquote", "cdecl")
    RKUnquote.argtypes = [String]
    RKUnquote.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 251
if _libs["radarkit"].has("RKIndentCopy", "cdecl"):
    RKIndentCopy = _libs["radarkit"].get("RKIndentCopy", "cdecl")
    RKIndentCopy.argtypes = [String, String, c_int]
    RKIndentCopy.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 252
if _libs["radarkit"].has("RKStringCenterized", "cdecl"):
    RKStringCenterized = _libs["radarkit"].get("RKStringCenterized", "cdecl")
    RKStringCenterized.argtypes = [String, String, c_int]
    RKStringCenterized.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 253
if _libs["radarkit"].has("RKNextNoneWhite", "cdecl"):
    RKNextNoneWhite = _libs["radarkit"].get("RKNextNoneWhite", "cdecl")
    RKNextNoneWhite.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKNextNoneWhite.restype = ReturnString
    else:
        RKNextNoneWhite.restype = String
        RKNextNoneWhite.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 254
if _libs["radarkit"].has("RKLastLine", "cdecl"):
    RKLastLine = _libs["radarkit"].get("RKLastLine", "cdecl")
    RKLastLine.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKLastLine.restype = ReturnString
    else:
        RKLastLine.restype = String
        RKLastLine.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 255
if _libs["radarkit"].has("RKStripEscapeSequence", "cdecl"):
    RKStripEscapeSequence = _libs["radarkit"].get("RKStripEscapeSequence", "cdecl")
    RKStripEscapeSequence.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStripEscapeSequence.restype = ReturnString
    else:
        RKStripEscapeSequence.restype = String
        RKStripEscapeSequence.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 261
if _libs["radarkit"].has("RKMinDiff", "cdecl"):
    RKMinDiff = _libs["radarkit"].get("RKMinDiff", "cdecl")
    RKMinDiff.argtypes = [c_float, c_float]
    RKMinDiff.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 262
if _libs["radarkit"].has("RKUMinDiff", "cdecl"):
    RKUMinDiff = _libs["radarkit"].get("RKUMinDiff", "cdecl")
    RKUMinDiff.argtypes = [c_float, c_float]
    RKUMinDiff.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 263
if _libs["radarkit"].has("RKModulo360Diff", "cdecl"):
    RKModulo360Diff = _libs["radarkit"].get("RKModulo360Diff", "cdecl")
    RKModulo360Diff.argtypes = [c_float, c_float]
    RKModulo360Diff.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 264
if _libs["radarkit"].has("RKAngularCrossOver", "cdecl"):
    RKAngularCrossOver = _libs["radarkit"].get("RKAngularCrossOver", "cdecl")
    RKAngularCrossOver.argtypes = [c_float, c_float, c_float]
    RKAngularCrossOver.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 270
if _libs["radarkit"].has("RKGetCPUIndex", "cdecl"):
    RKGetCPUIndex = _libs["radarkit"].get("RKGetCPUIndex", "cdecl")
    RKGetCPUIndex.argtypes = [c_long]
    RKGetCPUIndex.restype = c_long

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 271
if _libs["radarkit"].has("RKGetMemoryUsage", "cdecl"):
    RKGetMemoryUsage = _libs["radarkit"].get("RKGetMemoryUsage", "cdecl")
    RKGetMemoryUsage.argtypes = []
    RKGetMemoryUsage.restype = c_long

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 277
if _libs["radarkit"].has("RKCountryFromPosition", "cdecl"):
    RKCountryFromPosition = _libs["radarkit"].get("RKCountryFromPosition", "cdecl")
    RKCountryFromPosition.argtypes = [c_double, c_double]
    if sizeof(c_int) == sizeof(c_void_p):
        RKCountryFromPosition.restype = ReturnString
    else:
        RKCountryFromPosition.restype = String
        RKCountryFromPosition.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 283
if _libs["radarkit"].has("RKGetNextKeyValue", "cdecl"):
    RKGetNextKeyValue = _libs["radarkit"].get("RKGetNextKeyValue", "cdecl")
    RKGetNextKeyValue.argtypes = [String, String, String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKGetNextKeyValue.restype = ReturnString
    else:
        RKGetNextKeyValue.restype = String
        RKGetNextKeyValue.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 284
if _libs["radarkit"].has("RKMergeColumns", "cdecl"):
    RKMergeColumns = _libs["radarkit"].get("RKMergeColumns", "cdecl")
    RKMergeColumns.argtypes = [String, String, String, c_int]
    RKMergeColumns.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 290
if _libs["radarkit"].has("RKBinaryString", "cdecl"):
    RKBinaryString = _libs["radarkit"].get("RKBinaryString", "cdecl")
    RKBinaryString.argtypes = [String, POINTER(None), c_size_t]
    if sizeof(c_int) == sizeof(c_void_p):
        RKBinaryString.restype = ReturnString
    else:
        RKBinaryString.restype = String
        RKBinaryString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 291
if _libs["radarkit"].has("RKHeadTailBinaryString", "cdecl"):
    RKHeadTailBinaryString = _libs["radarkit"].get("RKHeadTailBinaryString", "cdecl")
    RKHeadTailBinaryString.argtypes = [String, POINTER(None), c_size_t]
    RKHeadTailBinaryString.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 292
if _libs["radarkit"].has("RKBytesInHex", "cdecl"):
    RKBytesInHex = _libs["radarkit"].get("RKBytesInHex", "cdecl")
    RKBytesInHex.argtypes = [String, POINTER(None), c_size_t]
    if sizeof(c_int) == sizeof(c_void_p):
        RKBytesInHex.restype = ReturnString
    else:
        RKBytesInHex.restype = String
        RKBytesInHex.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 293
if _libs["radarkit"].has("RKHeadTailBytesInHex", "cdecl"):
    RKHeadTailBytesInHex = _libs["radarkit"].get("RKHeadTailBytesInHex", "cdecl")
    RKHeadTailBytesInHex.argtypes = [String, POINTER(None), c_size_t]
    RKHeadTailBytesInHex.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 294
if _libs["radarkit"].has("RKRadarHubPayloadString", "cdecl"):
    RKRadarHubPayloadString = _libs["radarkit"].get("RKRadarHubPayloadString", "cdecl")
    RKRadarHubPayloadString.argtypes = [String, POINTER(None), c_size_t]
    RKRadarHubPayloadString.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 300
if _libs["radarkit"].has("RKStringLower", "cdecl"):
    RKStringLower = _libs["radarkit"].get("RKStringLower", "cdecl")
    RKStringLower.argtypes = [String]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringLower.restype = ReturnString
    else:
        RKStringLower.restype = String
        RKStringLower.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 92
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
    ('lock', pthread_mutex_t),
    ('stream', POINTER(FILE)),
]

RKGlobalParameters = struct_RKGlobalParameterStruct# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 92

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 94
try:
    rkGlobalParameters = (RKGlobalParameters).in_dll(_libs["radarkit"], "rkGlobalParameters")
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 95
try:
    rkResultStrings = (POINTER(POINTER(c_char))).in_dll(_libs["radarkit"], "rkResultStrings")
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 100
if _libs["radarkit"].has("RKComplexAdd", "cdecl"):
    RKComplexAdd = _libs["radarkit"].get("RKComplexAdd", "cdecl")
    RKComplexAdd.argtypes = [RKComplex, RKComplex]
    RKComplexAdd.restype = RKComplex

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 101
if _libs["radarkit"].has("RKComplexSubtract", "cdecl"):
    RKComplexSubtract = _libs["radarkit"].get("RKComplexSubtract", "cdecl")
    RKComplexSubtract.argtypes = [RKComplex, RKComplex]
    RKComplexSubtract.restype = RKComplex

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 102
if _libs["radarkit"].has("RKComplexMultiply", "cdecl"):
    RKComplexMultiply = _libs["radarkit"].get("RKComplexMultiply", "cdecl")
    RKComplexMultiply.argtypes = [RKComplex, RKComplex]
    RKComplexMultiply.restype = RKComplex

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 103
if _libs["radarkit"].has("RKComplexConjugate", "cdecl"):
    RKComplexConjugate = _libs["radarkit"].get("RKComplexConjugate", "cdecl")
    RKComplexConjugate.argtypes = [RKComplex]
    RKComplexConjugate.restype = RKComplex

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 104
if _libs["radarkit"].has("RKComplexAbsSquare", "cdecl"):
    RKComplexAbsSquare = _libs["radarkit"].get("RKComplexAbsSquare", "cdecl")
    RKComplexAbsSquare.argtypes = [RKComplex]
    RKComplexAbsSquare.restype = RKFloat

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 107
if _libs["radarkit"].has("RKComplexArrayInConjugate", "cdecl"):
    RKComplexArrayInConjugate = _libs["radarkit"].get("RKComplexArrayInConjugate", "cdecl")
    RKComplexArrayInConjugate.argtypes = [POINTER(RKComplex), c_int]
    RKComplexArrayInConjugate.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 108
if _libs["radarkit"].has("RKComplexArrayInPlaceAdd", "cdecl"):
    RKComplexArrayInPlaceAdd = _libs["radarkit"].get("RKComplexArrayInPlaceAdd", "cdecl")
    RKComplexArrayInPlaceAdd.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceAdd.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 109
if _libs["radarkit"].has("RKComplexArrayInPlaceSubtract", "cdecl"):
    RKComplexArrayInPlaceSubtract = _libs["radarkit"].get("RKComplexArrayInPlaceSubtract", "cdecl")
    RKComplexArrayInPlaceSubtract.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceSubtract.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 110
if _libs["radarkit"].has("RKComplexArrayInPlaceMultiply", "cdecl"):
    RKComplexArrayInPlaceMultiply = _libs["radarkit"].get("RKComplexArrayInPlaceMultiply", "cdecl")
    RKComplexArrayInPlaceMultiply.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceMultiply.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 111
if _libs["radarkit"].has("RKComplexArrayInPlaceConjugateMultiply", "cdecl"):
    RKComplexArrayInPlaceConjugateMultiply = _libs["radarkit"].get("RKComplexArrayInPlaceConjugateMultiply", "cdecl")
    RKComplexArrayInPlaceConjugateMultiply.argtypes = [POINTER(RKComplex), POINTER(RKComplex), c_int]
    RKComplexArrayInPlaceConjugateMultiply.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 114
if _libs["radarkit"].has("RKFloatArraySum", "cdecl"):
    RKFloatArraySum = _libs["radarkit"].get("RKFloatArraySum", "cdecl")
    RKFloatArraySum.argtypes = [POINTER(RKFloat), c_int]
    RKFloatArraySum.restype = RKFloat

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 115
if _libs["radarkit"].has("RKComplexArraySum", "cdecl"):
    RKComplexArraySum = _libs["radarkit"].get("RKComplexArraySum", "cdecl")
    RKComplexArraySum.argtypes = [POINTER(RKComplex), c_int]
    RKComplexArraySum.restype = RKComplex

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 118
if _libs["radarkit"].has("RKLog", "cdecl"):
    _func = _libs["radarkit"].get("RKLog", "cdecl")
    _restype = c_int
    _errcheck = None
    _argtypes = [String]
    RKLog = _variadic_function(_func,_restype,_argtypes,_errcheck)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 119
if _libs["radarkit"].has("RKExit", "cdecl"):
    RKExit = _libs["radarkit"].get("RKExit", "cdecl")
    RKExit.argtypes = [c_int]
    RKExit.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 122
if _libs["radarkit"].has("RKFileOpen", "cdecl"):
    RKFileOpen = _libs["radarkit"].get("RKFileOpen", "cdecl")
    RKFileOpen.argtypes = [String, String]
    RKFileOpen.restype = POINTER(FILE)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 123
if _libs["radarkit"].has("RKFileClose", "cdecl"):
    RKFileClose = _libs["radarkit"].get("RKFileClose", "cdecl")
    RKFileClose.argtypes = [POINTER(FILE)]
    RKFileClose.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 124
if _libs["radarkit"].has("RKFileTell", "cdecl"):
    RKFileTell = _libs["radarkit"].get("RKFileTell", "cdecl")
    RKFileTell.argtypes = [POINTER(FILE)]
    RKFileTell.restype = c_long

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 125
if _libs["radarkit"].has("RKFileGetSize", "cdecl"):
    RKFileGetSize = _libs["radarkit"].get("RKFileGetSize", "cdecl")
    RKFileGetSize.argtypes = [POINTER(FILE)]
    RKFileGetSize.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 128
if _libs["radarkit"].has("RKSetStatusColor", "cdecl"):
    RKSetStatusColor = _libs["radarkit"].get("RKSetStatusColor", "cdecl")
    RKSetStatusColor.argtypes = [c_bool]
    RKSetStatusColor.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 129
if _libs["radarkit"].has("RKSetWantColor", "cdecl"):
    RKSetWantColor = _libs["radarkit"].get("RKSetWantColor", "cdecl")
    RKSetWantColor.argtypes = [c_bool]
    RKSetWantColor.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 130
if _libs["radarkit"].has("RKSetWantScreenOutput", "cdecl"):
    RKSetWantScreenOutput = _libs["radarkit"].get("RKSetWantScreenOutput", "cdecl")
    RKSetWantScreenOutput.argtypes = [c_bool]
    RKSetWantScreenOutput.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 131
if _libs["radarkit"].has("RKSetUseDailyLog", "cdecl"):
    RKSetUseDailyLog = _libs["radarkit"].get("RKSetUseDailyLog", "cdecl")
    RKSetUseDailyLog.argtypes = [c_bool]
    RKSetUseDailyLog.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 132
if _libs["radarkit"].has("RKSetProgramName", "cdecl"):
    RKSetProgramName = _libs["radarkit"].get("RKSetProgramName", "cdecl")
    RKSetProgramName.argtypes = [String]
    RKSetProgramName.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 133
if _libs["radarkit"].has("RKSetRootFolder", "cdecl"):
    RKSetRootFolder = _libs["radarkit"].get("RKSetRootFolder", "cdecl")
    RKSetRootFolder.argtypes = [String]
    RKSetRootFolder.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 134
if _libs["radarkit"].has("RKSetLogfile", "cdecl"):
    RKSetLogfile = _libs["radarkit"].get("RKSetLogfile", "cdecl")
    RKSetLogfile.argtypes = [String]
    RKSetLogfile.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 135
if _libs["radarkit"].has("RKSetLogfileToDefault", "cdecl"):
    RKSetLogfileToDefault = _libs["radarkit"].get("RKSetLogfileToDefault", "cdecl")
    RKSetLogfileToDefault.argtypes = []
    RKSetLogfileToDefault.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 137
if _libs["radarkit"].has("RKVersionString", "cdecl"):
    RKVersionString = _libs["radarkit"].get("RKVersionString", "cdecl")
    RKVersionString.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        RKVersionString.restype = ReturnString
    else:
        RKVersionString.restype = String
        RKVersionString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 138
if _libs["radarkit"].has("RKGuessValueType", "cdecl"):
    RKGuessValueType = _libs["radarkit"].get("RKGuessValueType", "cdecl")
    RKGuessValueType.argtypes = [String]
    RKGuessValueType.restype = RKValueType

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 141
if _libs["radarkit"].has("RKGetSymbolFromFilename", "cdecl"):
    RKGetSymbolFromFilename = _libs["radarkit"].get("RKGetSymbolFromFilename", "cdecl")
    RKGetSymbolFromFilename.argtypes = [String, String]
    RKGetSymbolFromFilename.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 142
if _libs["radarkit"].has("RKGetPrefixFromFilename", "cdecl"):
    RKGetPrefixFromFilename = _libs["radarkit"].get("RKGetPrefixFromFilename", "cdecl")
    RKGetPrefixFromFilename.argtypes = [String, String]
    RKGetPrefixFromFilename.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 143
if _libs["radarkit"].has("RKListFilesWithSamePrefix", "cdecl"):
    RKListFilesWithSamePrefix = _libs["radarkit"].get("RKListFilesWithSamePrefix", "cdecl")
    RKListFilesWithSamePrefix.argtypes = [String, POINTER(c_char * int(1024))]
    RKListFilesWithSamePrefix.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 146
if _libs["radarkit"].has("RKShowName", "cdecl"):
    RKShowName = _libs["radarkit"].get("RKShowName", "cdecl")
    RKShowName.argtypes = []
    RKShowName.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 147
if _libs["radarkit"].has("RKShowTypeSizes", "cdecl"):
    RKShowTypeSizes = _libs["radarkit"].get("RKShowTypeSizes", "cdecl")
    RKShowTypeSizes.argtypes = []
    RKShowTypeSizes.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 148
if _libs["radarkit"].has("RKShowVecFloatLowPrecision", "cdecl"):
    RKShowVecFloatLowPrecision = _libs["radarkit"].get("RKShowVecFloatLowPrecision", "cdecl")
    RKShowVecFloatLowPrecision.argtypes = [String, POINTER(c_float), c_int]
    RKShowVecFloatLowPrecision.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 149
if _libs["radarkit"].has("RKShowVecFloat", "cdecl"):
    RKShowVecFloat = _libs["radarkit"].get("RKShowVecFloat", "cdecl")
    RKShowVecFloat.argtypes = [String, POINTER(c_float), c_int]
    RKShowVecFloat.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 150
if _libs["radarkit"].has("RKShowVecIQZ", "cdecl"):
    RKShowVecIQZ = _libs["radarkit"].get("RKShowVecIQZ", "cdecl")
    RKShowVecIQZ.argtypes = [String, POINTER(RKIQZ), c_int]
    RKShowVecIQZ.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 151
if _libs["radarkit"].has("RKShowVecComplex", "cdecl"):
    RKShowVecComplex = _libs["radarkit"].get("RKShowVecComplex", "cdecl")
    RKShowVecComplex.argtypes = [String, POINTER(RKComplex), c_int]
    RKShowVecComplex.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 152
if _libs["radarkit"].has("RKShowArray", "cdecl"):
    RKShowArray = _libs["radarkit"].get("RKShowArray", "cdecl")
    RKShowArray.argtypes = [POINTER(RKFloat), String, c_int, c_int]
    RKShowArray.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 153
if _libs["radarkit"].has("RKStringFromValue", "cdecl"):
    RKStringFromValue = _libs["radarkit"].get("RKStringFromValue", "cdecl")
    RKStringFromValue.argtypes = [POINTER(None), RKValueType]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringFromValue.restype = ReturnString
    else:
        RKStringFromValue.restype = String
        RKStringFromValue.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 154
if _libs["radarkit"].has("RKVariableInString", "cdecl"):
    RKVariableInString = _libs["radarkit"].get("RKVariableInString", "cdecl")
    RKVariableInString.argtypes = [String, POINTER(None), RKValueType]
    if sizeof(c_int) == sizeof(c_void_p):
        RKVariableInString.restype = ReturnString
    else:
        RKVariableInString.restype = String
        RKVariableInString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 155
if _libs["radarkit"].has("RKPrettyStringSizeEstimate", "cdecl"):
    RKPrettyStringSizeEstimate = _libs["radarkit"].get("RKPrettyStringSizeEstimate", "cdecl")
    RKPrettyStringSizeEstimate.argtypes = [String]
    RKPrettyStringSizeEstimate.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 156
if _libs["radarkit"].has("RKPrettyStringFromKeyValueString", "cdecl"):
    RKPrettyStringFromKeyValueString = _libs["radarkit"].get("RKPrettyStringFromKeyValueString", "cdecl")
    RKPrettyStringFromKeyValueString.argtypes = [String, String]
    RKPrettyStringFromKeyValueString.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 159
if _libs["radarkit"].has("RKZeroOutFloat", "cdecl"):
    RKZeroOutFloat = _libs["radarkit"].get("RKZeroOutFloat", "cdecl")
    RKZeroOutFloat.argtypes = [POINTER(RKFloat), uint32_t]
    RKZeroOutFloat.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 160
if _libs["radarkit"].has("RKZeroOutIQZ", "cdecl"):
    RKZeroOutIQZ = _libs["radarkit"].get("RKZeroOutIQZ", "cdecl")
    RKZeroOutIQZ.argtypes = [POINTER(RKIQZ), uint32_t]
    RKZeroOutIQZ.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 161
if _libs["radarkit"].has("RKZeroTailFloat", "cdecl"):
    RKZeroTailFloat = _libs["radarkit"].get("RKZeroTailFloat", "cdecl")
    RKZeroTailFloat.argtypes = [POINTER(RKFloat), uint32_t, uint32_t]
    RKZeroTailFloat.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 162
if _libs["radarkit"].has("RKZeroTailIQZ", "cdecl"):
    RKZeroTailIQZ = _libs["radarkit"].get("RKZeroTailIQZ", "cdecl")
    RKZeroTailIQZ.argtypes = [POINTER(RKIQZ), uint32_t, uint32_t]
    RKZeroTailIQZ.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 165
if _libs["radarkit"].has("RKPulseBufferAlloc", "cdecl"):
    RKPulseBufferAlloc = _libs["radarkit"].get("RKPulseBufferAlloc", "cdecl")
    RKPulseBufferAlloc.argtypes = [POINTER(RKBuffer), uint32_t, uint32_t]
    RKPulseBufferAlloc.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 166
if _libs["radarkit"].has("RKPulseBufferFree", "cdecl"):
    RKPulseBufferFree = _libs["radarkit"].get("RKPulseBufferFree", "cdecl")
    RKPulseBufferFree.argtypes = [RKBuffer]
    RKPulseBufferFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 167
if _libs["radarkit"].has("RKGetPulseFromBuffer", "cdecl"):
    RKGetPulseFromBuffer = _libs["radarkit"].get("RKGetPulseFromBuffer", "cdecl")
    RKGetPulseFromBuffer.argtypes = [RKBuffer, uint32_t]
    RKGetPulseFromBuffer.restype = POINTER(RKPulse)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 168
if _libs["radarkit"].has("RKGetInt16CDataFromPulse", "cdecl"):
    RKGetInt16CDataFromPulse = _libs["radarkit"].get("RKGetInt16CDataFromPulse", "cdecl")
    RKGetInt16CDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetInt16CDataFromPulse.restype = POINTER(RKInt16C)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 169
if _libs["radarkit"].has("RKGetComplexDataFromPulse", "cdecl"):
    RKGetComplexDataFromPulse = _libs["radarkit"].get("RKGetComplexDataFromPulse", "cdecl")
    RKGetComplexDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetComplexDataFromPulse.restype = POINTER(RKComplex)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 170
if _libs["radarkit"].has("RKGetSplitComplexDataFromPulse", "cdecl"):
    RKGetSplitComplexDataFromPulse = _libs["radarkit"].get("RKGetSplitComplexDataFromPulse", "cdecl")
    RKGetSplitComplexDataFromPulse.argtypes = [POINTER(RKPulse), uint32_t]
    RKGetSplitComplexDataFromPulse.restype = RKIQZ

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 171
if _libs["radarkit"].has("RKClearPulseBuffer", "cdecl"):
    RKClearPulseBuffer = _libs["radarkit"].get("RKClearPulseBuffer", "cdecl")
    RKClearPulseBuffer.argtypes = [RKBuffer, uint32_t]
    RKClearPulseBuffer.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 172
if _libs["radarkit"].has("RKReadPulseFromFileReference", "cdecl"):
    RKReadPulseFromFileReference = _libs["radarkit"].get("RKReadPulseFromFileReference", "cdecl")
    RKReadPulseFromFileReference.argtypes = [POINTER(RKPulse), POINTER(RKFileHeader), POINTER(FILE)]
    RKReadPulseFromFileReference.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 173
if _libs["radarkit"].has("RKGetVacantPulseFromBuffer", "cdecl"):
    RKGetVacantPulseFromBuffer = _libs["radarkit"].get("RKGetVacantPulseFromBuffer", "cdecl")
    RKGetVacantPulseFromBuffer.argtypes = [RKBuffer, POINTER(uint32_t), uint32_t]
    RKGetVacantPulseFromBuffer.restype = POINTER(RKPulse)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 176
if _libs["radarkit"].has("RKRayBufferAlloc", "cdecl"):
    RKRayBufferAlloc = _libs["radarkit"].get("RKRayBufferAlloc", "cdecl")
    RKRayBufferAlloc.argtypes = [POINTER(RKBuffer), uint32_t, uint32_t]
    RKRayBufferAlloc.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 177
if _libs["radarkit"].has("RKRayBufferFree", "cdecl"):
    RKRayBufferFree = _libs["radarkit"].get("RKRayBufferFree", "cdecl")
    RKRayBufferFree.argtypes = [RKBuffer]
    RKRayBufferFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 178
if _libs["radarkit"].has("RKGetRayFromBuffer", "cdecl"):
    RKGetRayFromBuffer = _libs["radarkit"].get("RKGetRayFromBuffer", "cdecl")
    RKGetRayFromBuffer.argtypes = [RKBuffer, uint32_t]
    RKGetRayFromBuffer.restype = POINTER(RKRay)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 179
if _libs["radarkit"].has("RKGetInt16DataFromRay", "cdecl"):
    RKGetInt16DataFromRay = _libs["radarkit"].get("RKGetInt16DataFromRay", "cdecl")
    RKGetInt16DataFromRay.argtypes = [POINTER(RKRay), RKMomentIndex]
    RKGetInt16DataFromRay.restype = POINTER(c_int16)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 180
if _libs["radarkit"].has("RKGetUInt8DataFromRay", "cdecl"):
    RKGetUInt8DataFromRay = _libs["radarkit"].get("RKGetUInt8DataFromRay", "cdecl")
    RKGetUInt8DataFromRay.argtypes = [POINTER(RKRay), RKBaseProductIndex]
    RKGetUInt8DataFromRay.restype = POINTER(uint8_t)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 181
if _libs["radarkit"].has("RKGetFloatDataFromRay", "cdecl"):
    RKGetFloatDataFromRay = _libs["radarkit"].get("RKGetFloatDataFromRay", "cdecl")
    RKGetFloatDataFromRay.argtypes = [POINTER(RKRay), RKBaseProductIndex]
    RKGetFloatDataFromRay.restype = POINTER(c_float)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 182
if _libs["radarkit"].has("RKClearRayBuffer", "cdecl"):
    RKClearRayBuffer = _libs["radarkit"].get("RKClearRayBuffer", "cdecl")
    RKClearRayBuffer.argtypes = [RKBuffer, uint32_t]
    RKClearRayBuffer.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 183
if _libs["radarkit"].has("RKGetVacantRayFromBuffer", "cdecl"):
    RKGetVacantRayFromBuffer = _libs["radarkit"].get("RKGetVacantRayFromBuffer", "cdecl")
    RKGetVacantRayFromBuffer.argtypes = [RKBuffer, POINTER(uint32_t), uint32_t]
    RKGetVacantRayFromBuffer.restype = POINTER(RKRay)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 186
if _libs["radarkit"].has("RKFileMonitorInit", "cdecl"):
    RKFileMonitorInit = _libs["radarkit"].get("RKFileMonitorInit", "cdecl")
    RKFileMonitorInit.argtypes = [String, CFUNCTYPE(UNCHECKED(None), POINTER(None)), POINTER(None)]
    RKFileMonitorInit.restype = POINTER(RKFileMonitor)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 187
if _libs["radarkit"].has("RKFileMonitorFree", "cdecl"):
    RKFileMonitorFree = _libs["radarkit"].get("RKFileMonitorFree", "cdecl")
    RKFileMonitorFree.argtypes = [POINTER(RKFileMonitor)]
    RKFileMonitorFree.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 190
if _libs["radarkit"].has("RKStreamFromString", "cdecl"):
    RKStreamFromString = _libs["radarkit"].get("RKStreamFromString", "cdecl")
    RKStreamFromString.argtypes = [String]
    RKStreamFromString.restype = RKStream

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 191
if _libs["radarkit"].has("RKStringOfStream", "cdecl"):
    RKStringOfStream = _libs["radarkit"].get("RKStringOfStream", "cdecl")
    RKStringOfStream.argtypes = [RKStream]
    if sizeof(c_int) == sizeof(c_void_p):
        RKStringOfStream.restype = ReturnString
    else:
        RKStringOfStream.restype = String
        RKStringOfStream.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 192
if _libs["radarkit"].has("RKStringFromStream", "cdecl"):
    RKStringFromStream = _libs["radarkit"].get("RKStringFromStream", "cdecl")
    RKStringFromStream.argtypes = [String, RKStream]
    RKStringFromStream.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 194
if _libs["radarkit"].has("RKGetNextProductDescription", "cdecl"):
    RKGetNextProductDescription = _libs["radarkit"].get("RKGetNextProductDescription", "cdecl")
    RKGetNextProductDescription.argtypes = [POINTER(RKBaseProductList)]
    RKGetNextProductDescription.restype = RKProductDesc

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 197
if _libs["radarkit"].has("RKParseCommaDelimitedValues", "cdecl"):
    RKParseCommaDelimitedValues = _libs["radarkit"].get("RKParseCommaDelimitedValues", "cdecl")
    RKParseCommaDelimitedValues.argtypes = [POINTER(None), RKValueType, c_size_t, String]
    RKParseCommaDelimitedValues.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 198
if _libs["radarkit"].has("RKParseNumericArray", "cdecl"):
    RKParseNumericArray = _libs["radarkit"].get("RKParseNumericArray", "cdecl")
    RKParseNumericArray.argtypes = [POINTER(None), RKValueType, c_size_t, String]
    RKParseNumericArray.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 199
if _libs["radarkit"].has("RKParseQuotedStrings", "cdecl"):
    _func = _libs["radarkit"].get("RKParseQuotedStrings", "cdecl")
    _restype = None
    _errcheck = None
    _argtypes = [String]
    RKParseQuotedStrings = _variadic_function(_func,_restype,_argtypes,_errcheck)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 200
if _libs["radarkit"].has("RKMakeJSONStringFromControls", "cdecl"):
    RKMakeJSONStringFromControls = _libs["radarkit"].get("RKMakeJSONStringFromControls", "cdecl")
    RKMakeJSONStringFromControls.argtypes = [String, POINTER(RKControl), uint32_t]
    RKMakeJSONStringFromControls.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 201
if _libs["radarkit"].has("RKValueToEnum", "cdecl"):
    RKValueToEnum = _libs["radarkit"].get("RKValueToEnum", "cdecl")
    RKValueToEnum.argtypes = [RKConst, RKConst, RKConst, RKConst, RKConst, RKConst, RKConst]
    RKValueToEnum.restype = RKStatusEnum

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 202
if _libs["radarkit"].has("RKStatusFromTemperatureForCE", "cdecl"):
    RKStatusFromTemperatureForCE = _libs["radarkit"].get("RKStatusFromTemperatureForCE", "cdecl")
    RKStatusFromTemperatureForCE.argtypes = [RKConst]
    RKStatusFromTemperatureForCE.restype = RKStatusEnum

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 203
if _libs["radarkit"].has("RKStatusFromTemperatureForIE", "cdecl"):
    RKStatusFromTemperatureForIE = _libs["radarkit"].get("RKStatusFromTemperatureForIE", "cdecl")
    RKStatusFromTemperatureForIE.argtypes = [RKConst]
    RKStatusFromTemperatureForIE.restype = RKStatusEnum

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 204
if _libs["radarkit"].has("RKStatusFromTemperatureForComputers", "cdecl"):
    RKStatusFromTemperatureForComputers = _libs["radarkit"].get("RKStatusFromTemperatureForComputers", "cdecl")
    RKStatusFromTemperatureForComputers.argtypes = [RKConst]
    RKStatusFromTemperatureForComputers.restype = RKStatusEnum

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 205
if _libs["radarkit"].has("RKFindCondition", "cdecl"):
    RKFindCondition = _libs["radarkit"].get("RKFindCondition", "cdecl")
    RKFindCondition.argtypes = [String, RKStatusEnum, c_bool, String, String]
    RKFindCondition.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 206
if _libs["radarkit"].has("RKAnyCritical", "cdecl"):
    RKAnyCritical = _libs["radarkit"].get("RKAnyCritical", "cdecl")
    RKAnyCritical.argtypes = [String, c_bool, String, String]
    RKAnyCritical.restype = c_bool

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 207
if _libs["radarkit"].has("RKParseProductDescription", "cdecl"):
    RKParseProductDescription = _libs["radarkit"].get("RKParseProductDescription", "cdecl")
    RKParseProductDescription.argtypes = [POINTER(RKProductDesc), String]
    RKParseProductDescription.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 208
if _libs["radarkit"].has("RKProductIdFromString", "cdecl"):
    RKProductIdFromString = _libs["radarkit"].get("RKProductIdFromString", "cdecl")
    RKProductIdFromString.argtypes = [String]
    RKProductIdFromString.restype = RKProductId

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 209
if _libs["radarkit"].has("RKIdentifierFromString", "cdecl"):
    RKIdentifierFromString = _libs["radarkit"].get("RKIdentifierFromString", "cdecl")
    RKIdentifierFromString.argtypes = [String]
    RKIdentifierFromString.restype = RKIdentifier

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 212
if _libs["radarkit"].has("RKSimpleEngineFree", "cdecl"):
    RKSimpleEngineFree = _libs["radarkit"].get("RKSimpleEngineFree", "cdecl")
    RKSimpleEngineFree.argtypes = [POINTER(RKSimpleEngine)]
    RKSimpleEngineFree.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 215
if _libs["radarkit"].has("RKCommandQueueInit", "cdecl"):
    RKCommandQueueInit = _libs["radarkit"].get("RKCommandQueueInit", "cdecl")
    RKCommandQueueInit.argtypes = [uint16_t, c_bool]
    RKCommandQueueInit.restype = POINTER(RKCommandQueue)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 216
if _libs["radarkit"].has("RKCommandQueuePop", "cdecl"):
    RKCommandQueuePop = _libs["radarkit"].get("RKCommandQueuePop", "cdecl")
    RKCommandQueuePop.argtypes = [POINTER(RKCommandQueue)]
    RKCommandQueuePop.restype = POINTER(RKCommand)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 217
if _libs["radarkit"].has("RKCommandQueuePush", "cdecl"):
    RKCommandQueuePush = _libs["radarkit"].get("RKCommandQueuePush", "cdecl")
    RKCommandQueuePush.argtypes = [POINTER(RKCommandQueue), POINTER(RKCommand)]
    RKCommandQueuePush.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 218
if _libs["radarkit"].has("RKCommandQueueFree", "cdecl"):
    RKCommandQueueFree = _libs["radarkit"].get("RKCommandQueueFree", "cdecl")
    RKCommandQueueFree.argtypes = [POINTER(RKCommandQueue)]
    RKCommandQueueFree.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 221
if _libs["radarkit"].has("RKPedestalActionString", "cdecl"):
    RKPedestalActionString = _libs["radarkit"].get("RKPedestalActionString", "cdecl")
    RKPedestalActionString.argtypes = [POINTER(RKScanAction)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPedestalActionString.restype = ReturnString
    else:
        RKPedestalActionString.restype = String
        RKPedestalActionString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKConfig.h: 14
if _libs["radarkit"].has("RKConfigBufferAlloc", "cdecl"):
    RKConfigBufferAlloc = _libs["radarkit"].get("RKConfigBufferAlloc", "cdecl")
    RKConfigBufferAlloc.argtypes = [POINTER(POINTER(RKConfig)), uint32_t]
    RKConfigBufferAlloc.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKConfig.h: 15
if _libs["radarkit"].has("RKConfigBufferFree", "cdecl"):
    RKConfigBufferFree = _libs["radarkit"].get("RKConfigBufferFree", "cdecl")
    RKConfigBufferFree.argtypes = [POINTER(RKConfig)]
    RKConfigBufferFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKConfig.h: 17
if _libs["radarkit"].has("RKConfigAdvanceEllipsis", "cdecl"):
    _func = _libs["radarkit"].get("RKConfigAdvanceEllipsis", "cdecl")
    _restype = None
    _errcheck = None
    _argtypes = [POINTER(RKConfig), POINTER(uint32_t), uint32_t]
    RKConfigAdvanceEllipsis = _variadic_function(_func,_restype,_argtypes,_errcheck)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKConfig.h: 18
if _libs["radarkit"].has("RKConfigAdvance", "cdecl"):
    RKConfigAdvance = _libs["radarkit"].get("RKConfigAdvance", "cdecl")
    RKConfigAdvance.argtypes = [POINTER(RKConfig), POINTER(uint32_t), uint32_t, c_void_p]
    RKConfigAdvance.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKConfig.h: 21
if _libs["radarkit"].has("RKConfigWithId", "cdecl"):
    RKConfigWithId = _libs["radarkit"].get("RKConfigWithId", "cdecl")
    RKConfigWithId.argtypes = [POINTER(RKConfig), uint32_t, uint64_t]
    RKConfigWithId.restype = POINTER(RKConfig)

RKWindowType = c_int# headers/RadarKit/RKWindow.h: 14

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 26
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

RKFFTResource = struct_rk_fft_resource# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 26

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 35
class struct_rk_fft_module(Structure):
    pass

struct_rk_fft_module.__slots__ = [
    'name',
    'verbose',
    'exportWisdom',
    'wisdomFile',
    'count',
    'plans',
]
struct_rk_fft_module._fields_ = [
    ('name', RKName),
    ('verbose', c_int),
    ('exportWisdom', c_bool),
    ('wisdomFile', c_char * int(64)),
    ('count', c_uint),
    ('plans', RKFFTResource * int(18)),
]

RKFFTModule = struct_rk_fft_module# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 35

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 41
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

RKGaussian = struct_rk_gaussian# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 41

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 47
if _libs["radarkit"].has("RKGetSignedMinorSectorInDegrees", "cdecl"):
    RKGetSignedMinorSectorInDegrees = _libs["radarkit"].get("RKGetSignedMinorSectorInDegrees", "cdecl")
    RKGetSignedMinorSectorInDegrees.argtypes = [c_float, c_float]
    RKGetSignedMinorSectorInDegrees.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 48
if _libs["radarkit"].has("RKGetMinorSectorInDegrees", "cdecl"):
    RKGetMinorSectorInDegrees = _libs["radarkit"].get("RKGetMinorSectorInDegrees", "cdecl")
    RKGetMinorSectorInDegrees.argtypes = [c_float, c_float]
    RKGetMinorSectorInDegrees.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 49
if _libs["radarkit"].has("RKInterpolatePositiveAngles", "cdecl"):
    RKInterpolatePositiveAngles = _libs["radarkit"].get("RKInterpolatePositiveAngles", "cdecl")
    RKInterpolatePositiveAngles.argtypes = [c_float, c_float, c_float]
    RKInterpolatePositiveAngles.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 50
if _libs["radarkit"].has("RKInterpolateAngles", "cdecl"):
    RKInterpolateAngles = _libs["radarkit"].get("RKInterpolateAngles", "cdecl")
    RKInterpolateAngles.argtypes = [c_float, c_float, c_float]
    RKInterpolateAngles.restype = c_float

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 52
if _libs["radarkit"].has("RKMeasureNoiseFromPulse", "cdecl"):
    RKMeasureNoiseFromPulse = _libs["radarkit"].get("RKMeasureNoiseFromPulse", "cdecl")
    RKMeasureNoiseFromPulse.argtypes = [POINTER(RKFloat), POINTER(RKPulse), c_int]
    RKMeasureNoiseFromPulse.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 53
if _libs["radarkit"].has("RKBestStrideOfHopsV1", "cdecl"):
    RKBestStrideOfHopsV1 = _libs["radarkit"].get("RKBestStrideOfHopsV1", "cdecl")
    RKBestStrideOfHopsV1.argtypes = [c_int, c_bool]
    RKBestStrideOfHopsV1.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 54
if _libs["radarkit"].has("RKBestStrideOfHops", "cdecl"):
    RKBestStrideOfHops = _libs["radarkit"].get("RKBestStrideOfHops", "cdecl")
    RKBestStrideOfHops.argtypes = [c_int, c_bool]
    RKBestStrideOfHops.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 56
if _libs["radarkit"].has("RKHilbertTransform", "cdecl"):
    RKHilbertTransform = _libs["radarkit"].get("RKHilbertTransform", "cdecl")
    RKHilbertTransform.argtypes = [POINTER(RKFloat), POINTER(RKComplex), c_int]
    RKHilbertTransform.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 58
if _libs["radarkit"].has("RKFasterSineCosine", "cdecl"):
    RKFasterSineCosine = _libs["radarkit"].get("RKFasterSineCosine", "cdecl")
    RKFasterSineCosine.argtypes = [c_float, POINTER(c_float), POINTER(c_float)]
    RKFasterSineCosine.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 59
if _libs["radarkit"].has("RKFastSineCosine", "cdecl"):
    RKFastSineCosine = _libs["radarkit"].get("RKFastSineCosine", "cdecl")
    RKFastSineCosine.argtypes = [c_float, POINTER(c_float), POINTER(c_float)]
    RKFastSineCosine.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 65
if _libs["radarkit"].has("RKGetFilterCoefficients", "cdecl"):
    RKGetFilterCoefficients = _libs["radarkit"].get("RKGetFilterCoefficients", "cdecl")
    RKGetFilterCoefficients.argtypes = [POINTER(RKIIRFilter), RKFilterType]
    RKGetFilterCoefficients.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 71
if _libs["radarkit"].has("RKFFTModuleInit", "cdecl"):
    RKFFTModuleInit = _libs["radarkit"].get("RKFFTModuleInit", "cdecl")
    RKFFTModuleInit.argtypes = [uint32_t, c_int]
    RKFFTModuleInit.restype = POINTER(RKFFTModule)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 72
if _libs["radarkit"].has("RKFFTModuleFree", "cdecl"):
    RKFFTModuleFree = _libs["radarkit"].get("RKFFTModuleFree", "cdecl")
    RKFFTModuleFree.argtypes = [POINTER(RKFFTModule)]
    RKFFTModuleFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 78
if _libs["radarkit"].has("RKSGFit", "cdecl"):
    RKSGFit = _libs["radarkit"].get("RKSGFit", "cdecl")
    RKSGFit.argtypes = [POINTER(RKFloat), POINTER(RKComplex), c_int]
    RKSGFit.restype = RKGaussian

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 84
if _libs["radarkit"].has("RKSingle2Double", "cdecl"):
    RKSingle2Double = _libs["radarkit"].get("RKSingle2Double", "cdecl")
    RKSingle2Double.argtypes = [RKWordFloat32]
    RKSingle2Double.restype = RKWordFloat64

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 85
if _libs["radarkit"].has("RKHalf2Single", "cdecl"):
    RKHalf2Single = _libs["radarkit"].get("RKHalf2Single", "cdecl")
    RKHalf2Single.argtypes = [RKWordFloat16]
    RKHalf2Single.restype = RKWordFloat32

RKCellMask = c_int8# headers/RadarKit/RKScratch.h: 15

enum_anon_164 = c_int# headers/RadarKit/RKScratch.h: 16

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

# headers/RadarKit/RKScratch.h: 110
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
    ('fftOrder', c_int8),
    ('config', POINTER(RKConfig)),
    ('calculatedMoments', RKMomentList),
    ('calculatedProducts', RKBaseProductList),
]

RKMomentScratch = struct_rk_moment_scratch# headers/RadarKit/RKScratch.h: 110

# headers/RadarKit/RKScratch.h: 112
if _libs["radarkit"].has("RKCompressionScratchAlloc", "cdecl"):
    RKCompressionScratchAlloc = _libs["radarkit"].get("RKCompressionScratchAlloc", "cdecl")
    RKCompressionScratchAlloc.argtypes = [POINTER(POINTER(RKCompressionScratch)), uint32_t, uint8_t, RKName]
    RKCompressionScratchAlloc.restype = c_size_t

# headers/RadarKit/RKScratch.h: 113
if _libs["radarkit"].has("RKCompressionScratchFree", "cdecl"):
    RKCompressionScratchFree = _libs["radarkit"].get("RKCompressionScratchFree", "cdecl")
    RKCompressionScratchFree.argtypes = [POINTER(RKCompressionScratch)]
    RKCompressionScratchFree.restype = None

# headers/RadarKit/RKScratch.h: 115
if _libs["radarkit"].has("RKMomentScratchAlloc", "cdecl"):
    RKMomentScratchAlloc = _libs["radarkit"].get("RKMomentScratchAlloc", "cdecl")
    RKMomentScratchAlloc.argtypes = [POINTER(POINTER(RKMomentScratch)), uint32_t, uint8_t, RKName]
    RKMomentScratchAlloc.restype = c_size_t

# headers/RadarKit/RKScratch.h: 116
if _libs["radarkit"].has("RKMomentScratchFree", "cdecl"):
    RKMomentScratchFree = _libs["radarkit"].get("RKMomentScratchFree", "cdecl")
    RKMomentScratchFree.argtypes = [POINTER(RKMomentScratch)]
    RKMomentScratchFree.restype = None

# headers/RadarKit/RKScratch.h: 118
if _libs["radarkit"].has("prepareScratch", "cdecl"):
    prepareScratch = _libs["radarkit"].get("prepareScratch", "cdecl")
    prepareScratch.argtypes = [POINTER(RKMomentScratch)]
    prepareScratch.restype = c_int

# headers/RadarKit/RKScratch.h: 119
if _libs["radarkit"].has("makeRayFromScratch", "cdecl"):
    makeRayFromScratch = _libs["radarkit"].get("makeRayFromScratch", "cdecl")
    makeRayFromScratch.argtypes = [POINTER(RKMomentScratch), POINTER(RKRay)]
    makeRayFromScratch.restype = c_int

# headers/RadarKit/RKScratch.h: 121
if _libs["radarkit"].has("RKNullProcessor", "cdecl"):
    RKNullProcessor = _libs["radarkit"].get("RKNullProcessor", "cdecl")
    RKNullProcessor.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKNullProcessor.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 22
class struct_rk_pulse_worker(Structure):
    pass

RKPulseWorker = struct_rk_pulse_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 17

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 40
class struct_rk_pulse_engine(Structure):
    pass

RKPulseEngine = struct_rk_pulse_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 18

RKPulseEnginePlanIndex = c_int * int(8)# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 20

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
    'fftModule',
    'userModule',
    'verbose',
    'coreCount',
    'coreOrigin',
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
    'statusBuffer',
    'pulseStatusBuffer',
    'statusBufferIndex',
    'pulseStatusBufferIndex',
    'state',
    'tic',
    'lag',
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
    ('fftModule', POINTER(RKFFTModule)),
    ('userModule', RKUserModule),
    ('verbose', uint8_t),
    ('coreCount', uint8_t),
    ('coreOrigin', uint8_t),
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
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('pulseStatusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('pulseStatusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('minWorkerLag', c_float),
    ('maxWorkerLag', c_float),
    ('almostFull', c_int),
    ('memoryUsage', c_size_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 83
if _libs["radarkit"].has("RKPulseEngineInit", "cdecl"):
    RKPulseEngineInit = _libs["radarkit"].get("RKPulseEngineInit", "cdecl")
    RKPulseEngineInit.argtypes = []
    RKPulseEngineInit.restype = POINTER(RKPulseEngine)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 84
if _libs["radarkit"].has("RKPulseEngineFree", "cdecl"):
    RKPulseEngineFree = _libs["radarkit"].get("RKPulseEngineFree", "cdecl")
    RKPulseEngineFree.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 86
if _libs["radarkit"].has("RKPulseEngineSetVerbose", "cdecl"):
    RKPulseEngineSetVerbose = _libs["radarkit"].get("RKPulseEngineSetVerbose", "cdecl")
    RKPulseEngineSetVerbose.argtypes = [POINTER(RKPulseEngine), c_int]
    RKPulseEngineSetVerbose.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 87
if _libs["radarkit"].has("RKPulseEngineSetEssentials", "cdecl"):
    RKPulseEngineSetEssentials = _libs["radarkit"].get("RKPulseEngineSetEssentials", "cdecl")
    RKPulseEngineSetEssentials.argtypes = [POINTER(RKPulseEngine), POINTER(RKRadarDesc), POINTER(RKFFTModule), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseEngineSetEssentials.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 90
if _libs["radarkit"].has("RKPulseEngineSetInputOutputBuffers", "cdecl"):
    RKPulseEngineSetInputOutputBuffers = _libs["radarkit"].get("RKPulseEngineSetInputOutputBuffers", "cdecl")
    RKPulseEngineSetInputOutputBuffers.argtypes = [POINTER(RKPulseEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseEngineSetInputOutputBuffers.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 94
if _libs["radarkit"].has("RKPulseEngineSetFFTModule", "cdecl"):
    RKPulseEngineSetFFTModule = _libs["radarkit"].get("RKPulseEngineSetFFTModule", "cdecl")
    RKPulseEngineSetFFTModule.argtypes = [POINTER(RKPulseEngine), POINTER(RKFFTModule)]
    RKPulseEngineSetFFTModule.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 95
if _libs["radarkit"].has("RKPulseEngineSetCompressor", "cdecl"):
    RKPulseEngineSetCompressor = _libs["radarkit"].get("RKPulseEngineSetCompressor", "cdecl")
    RKPulseEngineSetCompressor.argtypes = [POINTER(RKPulseEngine), CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKCompressionScratch)), RKUserModule]
    RKPulseEngineSetCompressor.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 96
if _libs["radarkit"].has("RKPulseEngineSetCoreCount", "cdecl"):
    RKPulseEngineSetCoreCount = _libs["radarkit"].get("RKPulseEngineSetCoreCount", "cdecl")
    RKPulseEngineSetCoreCount.argtypes = [POINTER(RKPulseEngine), uint8_t]
    RKPulseEngineSetCoreCount.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 97
if _libs["radarkit"].has("RKPulseEngineSetCoreOrigin", "cdecl"):
    RKPulseEngineSetCoreOrigin = _libs["radarkit"].get("RKPulseEngineSetCoreOrigin", "cdecl")
    RKPulseEngineSetCoreOrigin.argtypes = [POINTER(RKPulseEngine), uint8_t]
    RKPulseEngineSetCoreOrigin.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 99
if _libs["radarkit"].has("RKPulseEngineResetFilters", "cdecl"):
    RKPulseEngineResetFilters = _libs["radarkit"].get("RKPulseEngineResetFilters", "cdecl")
    RKPulseEngineResetFilters.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineResetFilters.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 100
if _libs["radarkit"].has("RKPulseEngineSetFilterCountOfGroup", "cdecl"):
    RKPulseEngineSetFilterCountOfGroup = _libs["radarkit"].get("RKPulseEngineSetFilterCountOfGroup", "cdecl")
    RKPulseEngineSetFilterCountOfGroup.argtypes = [POINTER(RKPulseEngine), c_int, c_int]
    RKPulseEngineSetFilterCountOfGroup.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 101
if _libs["radarkit"].has("RKPulseEngineSetFilterGroupCount", "cdecl"):
    RKPulseEngineSetFilterGroupCount = _libs["radarkit"].get("RKPulseEngineSetFilterGroupCount", "cdecl")
    RKPulseEngineSetFilterGroupCount.argtypes = [POINTER(RKPulseEngine), c_int]
    RKPulseEngineSetFilterGroupCount.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 102
if _libs["radarkit"].has("RKPulseEngineSetGroupFilter", "cdecl"):
    RKPulseEngineSetGroupFilter = _libs["radarkit"].get("RKPulseEngineSetGroupFilter", "cdecl")
    RKPulseEngineSetGroupFilter.argtypes = [POINTER(RKPulseEngine), POINTER(RKComplex), RKFilterAnchor, c_int, c_int]
    RKPulseEngineSetGroupFilter.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 107
if _libs["radarkit"].has("RKPulseEngineSetFilter", "cdecl"):
    RKPulseEngineSetFilter = _libs["radarkit"].get("RKPulseEngineSetFilter", "cdecl")
    RKPulseEngineSetFilter.argtypes = [POINTER(RKPulseEngine), POINTER(RKComplex), RKFilterAnchor]
    RKPulseEngineSetFilter.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 108
if _libs["radarkit"].has("RKPulseEngineSetFilterByWaveform", "cdecl"):
    RKPulseEngineSetFilterByWaveform = _libs["radarkit"].get("RKPulseEngineSetFilterByWaveform", "cdecl")
    RKPulseEngineSetFilterByWaveform.argtypes = [POINTER(RKPulseEngine), POINTER(RKWaveform)]
    RKPulseEngineSetFilterByWaveform.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 109
if _libs["radarkit"].has("RKPulseEngineSetFilterToImpulse", "cdecl"):
    RKPulseEngineSetFilterToImpulse = _libs["radarkit"].get("RKPulseEngineSetFilterToImpulse", "cdecl")
    RKPulseEngineSetFilterToImpulse.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineSetFilterToImpulse.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 111
if _libs["radarkit"].has("RKPulseEngineStart", "cdecl"):
    RKPulseEngineStart = _libs["radarkit"].get("RKPulseEngineStart", "cdecl")
    RKPulseEngineStart.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineStart.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 112
if _libs["radarkit"].has("RKPulseEngineStop", "cdecl"):
    RKPulseEngineStop = _libs["radarkit"].get("RKPulseEngineStop", "cdecl")
    RKPulseEngineStop.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineStop.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 114
if _libs["radarkit"].has("RKPulseEngineGetVacantPulse", "cdecl"):
    RKPulseEngineGetVacantPulse = _libs["radarkit"].get("RKPulseEngineGetVacantPulse", "cdecl")
    RKPulseEngineGetVacantPulse.argtypes = [POINTER(RKPulseEngine), RKPulseStatus]
    RKPulseEngineGetVacantPulse.restype = POINTER(RKPulse)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 115
if _libs["radarkit"].has("RKPulseEngineGetProcessedPulse", "cdecl"):
    RKPulseEngineGetProcessedPulse = _libs["radarkit"].get("RKPulseEngineGetProcessedPulse", "cdecl")
    RKPulseEngineGetProcessedPulse.argtypes = [POINTER(RKPulseEngine), c_bool]
    RKPulseEngineGetProcessedPulse.restype = POINTER(RKPulse)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 116
if _libs["radarkit"].has("RKPulseEngineWaitWhileBusy", "cdecl"):
    RKPulseEngineWaitWhileBusy = _libs["radarkit"].get("RKPulseEngineWaitWhileBusy", "cdecl")
    RKPulseEngineWaitWhileBusy.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineWaitWhileBusy.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 118
if _libs["radarkit"].has("RKPulseEngineStatusString", "cdecl"):
    RKPulseEngineStatusString = _libs["radarkit"].get("RKPulseEngineStatusString", "cdecl")
    RKPulseEngineStatusString.argtypes = [POINTER(RKPulseEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseEngineStatusString.restype = ReturnString
    else:
        RKPulseEngineStatusString.restype = String
        RKPulseEngineStatusString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 119
if _libs["radarkit"].has("RKPulseEnginePulseString", "cdecl"):
    RKPulseEnginePulseString = _libs["radarkit"].get("RKPulseEnginePulseString", "cdecl")
    RKPulseEnginePulseString.argtypes = [POINTER(RKPulseEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseEnginePulseString.restype = ReturnString
    else:
        RKPulseEnginePulseString.restype = String
        RKPulseEnginePulseString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 120
if _libs["radarkit"].has("RKPulseEngineFilterSummary", "cdecl"):
    RKPulseEngineFilterSummary = _libs["radarkit"].get("RKPulseEngineFilterSummary", "cdecl")
    RKPulseEngineFilterSummary.argtypes = [POINTER(RKPulseEngine)]
    RKPulseEngineFilterSummary.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 122
if _libs["radarkit"].has("RKBuiltInCompressor", "cdecl"):
    RKBuiltInCompressor = _libs["radarkit"].get("RKBuiltInCompressor", "cdecl")
    RKBuiltInCompressor.argtypes = [RKUserModule, POINTER(RKCompressionScratch)]
    RKBuiltInCompressor.restype = None

# headers/RadarKit/RKWaveform.h: 18
if _libs["radarkit"].has("RKWaveformInitWithCountAndDepth", "cdecl"):
    RKWaveformInitWithCountAndDepth = _libs["radarkit"].get("RKWaveformInitWithCountAndDepth", "cdecl")
    RKWaveformInitWithCountAndDepth.argtypes = [c_int, c_int]
    RKWaveformInitWithCountAndDepth.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 19
if _libs["radarkit"].has("RKWaveformInitFromSamples", "cdecl"):
    RKWaveformInitFromSamples = _libs["radarkit"].get("RKWaveformInitFromSamples", "cdecl")
    RKWaveformInitFromSamples.argtypes = [POINTER(RKComplex), c_int, RKName]
    RKWaveformInitFromSamples.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 20
if _libs["radarkit"].has("RKWaveformInitFromFile", "cdecl"):
    RKWaveformInitFromFile = _libs["radarkit"].get("RKWaveformInitFromFile", "cdecl")
    RKWaveformInitFromFile.argtypes = [String]
    RKWaveformInitFromFile.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 21
if _libs["radarkit"].has("RKWaveformInit", "cdecl"):
    RKWaveformInit = _libs["radarkit"].get("RKWaveformInit", "cdecl")
    RKWaveformInit.argtypes = []
    RKWaveformInit.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 22
if _libs["radarkit"].has("RKWaveformFree", "cdecl"):
    RKWaveformFree = _libs["radarkit"].get("RKWaveformFree", "cdecl")
    RKWaveformFree.argtypes = [POINTER(RKWaveform)]
    RKWaveformFree.restype = None

# headers/RadarKit/RKWaveform.h: 24
if _libs["radarkit"].has("RKWaveformCopy", "cdecl"):
    RKWaveformCopy = _libs["radarkit"].get("RKWaveformCopy", "cdecl")
    RKWaveformCopy.argtypes = [POINTER(RKWaveform)]
    RKWaveformCopy.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 26
if _libs["radarkit"].has("RKWaveformInitAsImpulse", "cdecl"):
    RKWaveformInitAsImpulse = _libs["radarkit"].get("RKWaveformInitAsImpulse", "cdecl")
    RKWaveformInitAsImpulse.argtypes = []
    RKWaveformInitAsImpulse.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 27
if _libs["radarkit"].has("RKWaveformInitAsSingleTone", "cdecl"):
    RKWaveformInitAsSingleTone = _libs["radarkit"].get("RKWaveformInitAsSingleTone", "cdecl")
    RKWaveformInitAsSingleTone.argtypes = [c_double, c_double, c_double]
    RKWaveformInitAsSingleTone.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 28
if _libs["radarkit"].has("RKWaveformInitAsLinearFrequencyModulation", "cdecl"):
    RKWaveformInitAsLinearFrequencyModulation = _libs["radarkit"].get("RKWaveformInitAsLinearFrequencyModulation", "cdecl")
    RKWaveformInitAsLinearFrequencyModulation.argtypes = [c_double, c_double, c_double, c_double]
    RKWaveformInitAsLinearFrequencyModulation.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 29
if _libs["radarkit"].has("RKWaveformInitAsFrequencyHops", "cdecl"):
    RKWaveformInitAsFrequencyHops = _libs["radarkit"].get("RKWaveformInitAsFrequencyHops", "cdecl")
    RKWaveformInitAsFrequencyHops.argtypes = [c_double, c_double, c_double, c_double, c_int]
    RKWaveformInitAsFrequencyHops.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 30
if _libs["radarkit"].has("RKWaveformInitAsFakeTimeFrequencyMultiplexing", "cdecl"):
    RKWaveformInitAsFakeTimeFrequencyMultiplexing = _libs["radarkit"].get("RKWaveformInitAsFakeTimeFrequencyMultiplexing", "cdecl")
    RKWaveformInitAsFakeTimeFrequencyMultiplexing.argtypes = []
    RKWaveformInitAsFakeTimeFrequencyMultiplexing.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 31
if _libs["radarkit"].has("RKWaveformInitAsTimeFrequencyMultiplexing", "cdecl"):
    RKWaveformInitAsTimeFrequencyMultiplexing = _libs["radarkit"].get("RKWaveformInitAsTimeFrequencyMultiplexing", "cdecl")
    RKWaveformInitAsTimeFrequencyMultiplexing.argtypes = [c_double, c_double, c_double, c_double]
    RKWaveformInitAsTimeFrequencyMultiplexing.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 32
if _libs["radarkit"].has("RKWaveformInitAsFrequencyHoppingChirp", "cdecl"):
    RKWaveformInitAsFrequencyHoppingChirp = _libs["radarkit"].get("RKWaveformInitAsFrequencyHoppingChirp", "cdecl")
    RKWaveformInitAsFrequencyHoppingChirp.argtypes = [c_double, c_double, c_double, c_double, c_int]
    RKWaveformInitAsFrequencyHoppingChirp.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 33
if _libs["radarkit"].has("RKWaveformInitFromString", "cdecl"):
    RKWaveformInitFromString = _libs["radarkit"].get("RKWaveformInitFromString", "cdecl")
    RKWaveformInitFromString.argtypes = [String]
    RKWaveformInitFromString.restype = POINTER(RKWaveform)

# headers/RadarKit/RKWaveform.h: 35
if _libs["radarkit"].has("RKWaveformAppendWaveform", "cdecl"):
    RKWaveformAppendWaveform = _libs["radarkit"].get("RKWaveformAppendWaveform", "cdecl")
    RKWaveformAppendWaveform.argtypes = [POINTER(RKWaveform), POINTER(RKWaveform), uint32_t]
    RKWaveformAppendWaveform.restype = RKResult

# headers/RadarKit/RKWaveform.h: 36
if _libs["radarkit"].has("RKWaveformApplyWindow", "cdecl"):
    _func = _libs["radarkit"].get("RKWaveformApplyWindow", "cdecl")
    _restype = RKResult
    _errcheck = None
    _argtypes = [POINTER(RKWaveform), RKWindowType]
    RKWaveformApplyWindow = _variadic_function(_func,_restype,_argtypes,_errcheck)

# headers/RadarKit/RKWaveform.h: 38
if _libs["radarkit"].has("RKWaveformOnes", "cdecl"):
    RKWaveformOnes = _libs["radarkit"].get("RKWaveformOnes", "cdecl")
    RKWaveformOnes.argtypes = [POINTER(RKWaveform)]
    RKWaveformOnes.restype = None

# headers/RadarKit/RKWaveform.h: 39
if _libs["radarkit"].has("RKWaveformSingleTone", "cdecl"):
    RKWaveformSingleTone = _libs["radarkit"].get("RKWaveformSingleTone", "cdecl")
    RKWaveformSingleTone.argtypes = [POINTER(RKWaveform), c_double, c_double]
    RKWaveformSingleTone.restype = None

# headers/RadarKit/RKWaveform.h: 40
if _libs["radarkit"].has("RKWaveformLinearFrequencyModulation", "cdecl"):
    RKWaveformLinearFrequencyModulation = _libs["radarkit"].get("RKWaveformLinearFrequencyModulation", "cdecl")
    RKWaveformLinearFrequencyModulation.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformLinearFrequencyModulation.restype = None

# headers/RadarKit/RKWaveform.h: 41
if _libs["radarkit"].has("RKWaveformFrequencyHops", "cdecl"):
    RKWaveformFrequencyHops = _libs["radarkit"].get("RKWaveformFrequencyHops", "cdecl")
    RKWaveformFrequencyHops.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformFrequencyHops.restype = None

# headers/RadarKit/RKWaveform.h: 42
if _libs["radarkit"].has("RKWaveformFrequencyHoppingChirp", "cdecl"):
    RKWaveformFrequencyHoppingChirp = _libs["radarkit"].get("RKWaveformFrequencyHoppingChirp", "cdecl")
    RKWaveformFrequencyHoppingChirp.argtypes = [POINTER(RKWaveform), c_double, c_double, c_double]
    RKWaveformFrequencyHoppingChirp.restype = None

# headers/RadarKit/RKWaveform.h: 44
if _libs["radarkit"].has("RKWaveformDecimate", "cdecl"):
    RKWaveformDecimate = _libs["radarkit"].get("RKWaveformDecimate", "cdecl")
    RKWaveformDecimate.argtypes = [POINTER(RKWaveform), c_int]
    RKWaveformDecimate.restype = None

# headers/RadarKit/RKWaveform.h: 45
if _libs["radarkit"].has("RKWaveformConjuate", "cdecl"):
    RKWaveformConjuate = _libs["radarkit"].get("RKWaveformConjuate", "cdecl")
    RKWaveformConjuate.argtypes = [POINTER(RKWaveform)]
    RKWaveformConjuate.restype = None

# headers/RadarKit/RKWaveform.h: 46
if _libs["radarkit"].has("RKWaveformDownConvert", "cdecl"):
    RKWaveformDownConvert = _libs["radarkit"].get("RKWaveformDownConvert", "cdecl")
    RKWaveformDownConvert.argtypes = [POINTER(RKWaveform)]
    RKWaveformDownConvert.restype = None

# headers/RadarKit/RKWaveform.h: 48
if _libs["radarkit"].has("RKWaveformNormalizeNoiseGain", "cdecl"):
    RKWaveformNormalizeNoiseGain = _libs["radarkit"].get("RKWaveformNormalizeNoiseGain", "cdecl")
    RKWaveformNormalizeNoiseGain.argtypes = [POINTER(RKWaveform)]
    RKWaveformNormalizeNoiseGain.restype = None

# headers/RadarKit/RKWaveform.h: 49
if _libs["radarkit"].has("RKWaveformSummary", "cdecl"):
    RKWaveformSummary = _libs["radarkit"].get("RKWaveformSummary", "cdecl")
    RKWaveformSummary.argtypes = [POINTER(RKWaveform)]
    RKWaveformSummary.restype = None

# headers/RadarKit/RKWaveform.h: 51
if _libs["radarkit"].has("RKWaveformWriteToReference", "cdecl"):
    RKWaveformWriteToReference = _libs["radarkit"].get("RKWaveformWriteToReference", "cdecl")
    RKWaveformWriteToReference.argtypes = [POINTER(RKWaveform), POINTER(FILE)]
    RKWaveformWriteToReference.restype = c_size_t

# headers/RadarKit/RKWaveform.h: 52
if _libs["radarkit"].has("RKWaveformWriteFile", "cdecl"):
    RKWaveformWriteFile = _libs["radarkit"].get("RKWaveformWriteFile", "cdecl")
    RKWaveformWriteFile.argtypes = [POINTER(RKWaveform), String]
    RKWaveformWriteFile.restype = RKResult

# headers/RadarKit/RKWaveform.h: 53
if _libs["radarkit"].has("RKWaveformReadFromReference", "cdecl"):
    RKWaveformReadFromReference = _libs["radarkit"].get("RKWaveformReadFromReference", "cdecl")
    RKWaveformReadFromReference.argtypes = [POINTER(FILE)]
    RKWaveformReadFromReference.restype = POINTER(RKWaveform)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFileHeader.h: 15
if _libs["radarkit"].has("RKFileHeaderAlloc", "cdecl"):
    RKFileHeaderAlloc = _libs["radarkit"].get("RKFileHeaderAlloc", "cdecl")
    RKFileHeaderAlloc.argtypes = []
    RKFileHeaderAlloc.restype = POINTER(RKFileHeader)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFileHeader.h: 16
if _libs["radarkit"].has("RKFileHeaderFree", "cdecl"):
    RKFileHeaderFree = _libs["radarkit"].get("RKFileHeaderFree", "cdecl")
    RKFileHeaderFree.argtypes = [POINTER(RKFileHeader)]
    RKFileHeaderFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFileHeader.h: 18
if _libs["radarkit"].has("RKFileHeaderRead", "cdecl"):
    RKFileHeaderRead = _libs["radarkit"].get("RKFileHeaderRead", "cdecl")
    RKFileHeaderRead.argtypes = [POINTER(FILE)]
    RKFileHeaderRead.restype = POINTER(RKFileHeader)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFileHeader.h: 20
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
    ('path', c_char * int((768 + 32))),
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 20
class struct_rk_data_recorder(Structure):
    pass

RKRawDataRecorder = struct_rk_data_recorder# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 18

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
    'fd',
    'fid',
    'cache',
    'cacheWriteIndex',
    'cacheFlushCount',
    'fileWriteCount',
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
    ('fd', c_int),
    ('fid', POINTER(FILE)),
    ('cache', POINTER(None)),
    ('cacheWriteIndex', c_size_t),
    ('cacheFlushCount', uint64_t),
    ('fileWriteCount', uint64_t),
    ('tidPulseRecorder', pthread_t),
    ('statusBuffer', (c_char * int(256)) * int(10)),
    ('statusBufferIndex', uint32_t),
    ('state', RKEngineState),
    ('tic', uint64_t),
    ('lag', c_float),
    ('memoryUsage', c_size_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 53
if _libs["radarkit"].has("RKRawDataRecorderInit", "cdecl"):
    RKRawDataRecorderInit = _libs["radarkit"].get("RKRawDataRecorderInit", "cdecl")
    RKRawDataRecorderInit.argtypes = []
    RKRawDataRecorderInit.restype = POINTER(RKRawDataRecorder)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 54
if _libs["radarkit"].has("RKRawDataRecorderFree", "cdecl"):
    RKRawDataRecorderFree = _libs["radarkit"].get("RKRawDataRecorderFree", "cdecl")
    RKRawDataRecorderFree.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 56
if _libs["radarkit"].has("RKRawDataRecorderSetVerbose", "cdecl"):
    RKRawDataRecorderSetVerbose = _libs["radarkit"].get("RKRawDataRecorderSetVerbose", "cdecl")
    RKRawDataRecorderSetVerbose.argtypes = [POINTER(RKRawDataRecorder), c_int]
    RKRawDataRecorderSetVerbose.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 57
if _libs["radarkit"].has("RKRawDataRecorderSetEssentials", "cdecl"):
    RKRawDataRecorderSetEssentials = _libs["radarkit"].get("RKRawDataRecorderSetEssentials", "cdecl")
    RKRawDataRecorderSetEssentials.argtypes = [POINTER(RKRawDataRecorder), POINTER(RKRadarDesc), POINTER(RKFileManager), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKRawDataRecorderSetEssentials.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 60
if _libs["radarkit"].has("RKRawDataRecorderSetRecord", "cdecl"):
    RKRawDataRecorderSetRecord = _libs["radarkit"].get("RKRawDataRecorderSetRecord", "cdecl")
    RKRawDataRecorderSetRecord.argtypes = [POINTER(RKRawDataRecorder), c_bool]
    RKRawDataRecorderSetRecord.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 61
if _libs["radarkit"].has("RKRawDataRecorderSetRawDataType", "cdecl"):
    RKRawDataRecorderSetRawDataType = _libs["radarkit"].get("RKRawDataRecorderSetRawDataType", "cdecl")
    RKRawDataRecorderSetRawDataType.argtypes = [POINTER(RKRawDataRecorder), RKRawDataType]
    RKRawDataRecorderSetRawDataType.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 62
if _libs["radarkit"].has("RKRawDataRecorderSetMaximumRecordDepth", "cdecl"):
    RKRawDataRecorderSetMaximumRecordDepth = _libs["radarkit"].get("RKRawDataRecorderSetMaximumRecordDepth", "cdecl")
    RKRawDataRecorderSetMaximumRecordDepth.argtypes = [POINTER(RKRawDataRecorder), uint32_t]
    RKRawDataRecorderSetMaximumRecordDepth.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 63
if _libs["radarkit"].has("RKRawDataRecorderSetCacheSize", "cdecl"):
    RKRawDataRecorderSetCacheSize = _libs["radarkit"].get("RKRawDataRecorderSetCacheSize", "cdecl")
    RKRawDataRecorderSetCacheSize.argtypes = [POINTER(RKRawDataRecorder), uint32_t]
    RKRawDataRecorderSetCacheSize.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 65
if _libs["radarkit"].has("RKRawDataRecorderStart", "cdecl"):
    RKRawDataRecorderStart = _libs["radarkit"].get("RKRawDataRecorderStart", "cdecl")
    RKRawDataRecorderStart.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderStart.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 66
if _libs["radarkit"].has("RKRawDataRecorderStop", "cdecl"):
    RKRawDataRecorderStop = _libs["radarkit"].get("RKRawDataRecorderStop", "cdecl")
    RKRawDataRecorderStop.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderStop.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 67
if _libs["radarkit"].has("RKRawDataRecorderStatusString", "cdecl"):
    RKRawDataRecorderStatusString = _libs["radarkit"].get("RKRawDataRecorderStatusString", "cdecl")
    RKRawDataRecorderStatusString.argtypes = [POINTER(RKRawDataRecorder)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKRawDataRecorderStatusString.restype = ReturnString
    else:
        RKRawDataRecorderStatusString.restype = String
        RKRawDataRecorderStatusString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 69
if _libs["radarkit"].has("RKRawDataRecorderCacheWrite", "cdecl"):
    RKRawDataRecorderCacheWrite = _libs["radarkit"].get("RKRawDataRecorderCacheWrite", "cdecl")
    RKRawDataRecorderCacheWrite.argtypes = [POINTER(RKRawDataRecorder), POINTER(None), c_size_t]
    RKRawDataRecorderCacheWrite.restype = c_size_t

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 70
if _libs["radarkit"].has("RKRawDataRecorderCacheFlush", "cdecl"):
    RKRawDataRecorderCacheFlush = _libs["radarkit"].get("RKRawDataRecorderCacheFlush", "cdecl")
    RKRawDataRecorderCacheFlush.argtypes = [POINTER(RKRawDataRecorder)]
    RKRawDataRecorderCacheFlush.restype = c_size_t

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

# headers/RadarKit/RKPulseATSR.h: 15
if _libs["radarkit"].has("RKPulseATSR", "cdecl"):
    RKPulseATSR = _libs["radarkit"].get("RKPulseATSR", "cdecl")
    RKPulseATSR.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKPulseATSR.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 26
class struct_rk_moment_worker(Structure):
    pass

RKMomentWorker = struct_rk_moment_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 23

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 41
class struct_rk_moment_engine(Structure):
    pass

RKMomentEngine = struct_rk_moment_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 24

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
    'fftModule',
    'userModule',
    'verbose',
    'coreCount',
    'coreOrigin',
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
    ('fftModule', POINTER(RKFFTModule)),
    ('userModule', POINTER(RKUserModule)),
    ('verbose', uint8_t),
    ('coreCount', uint8_t),
    ('coreOrigin', uint8_t),
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 88
if _libs["radarkit"].has("RKMomentEngineInit", "cdecl"):
    RKMomentEngineInit = _libs["radarkit"].get("RKMomentEngineInit", "cdecl")
    RKMomentEngineInit.argtypes = []
    RKMomentEngineInit.restype = POINTER(RKMomentEngine)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 89
if _libs["radarkit"].has("RKMomentEngineFree", "cdecl"):
    RKMomentEngineFree = _libs["radarkit"].get("RKMomentEngineFree", "cdecl")
    RKMomentEngineFree.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 91
if _libs["radarkit"].has("RKMomentEngineSetVerbose", "cdecl"):
    RKMomentEngineSetVerbose = _libs["radarkit"].get("RKMomentEngineSetVerbose", "cdecl")
    RKMomentEngineSetVerbose.argtypes = [POINTER(RKMomentEngine), c_int]
    RKMomentEngineSetVerbose.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 92
if _libs["radarkit"].has("RKMomentEngineSetEssentials", "cdecl"):
    RKMomentEngineSetEssentials = _libs["radarkit"].get("RKMomentEngineSetEssentials", "cdecl")
    RKMomentEngineSetEssentials.argtypes = [POINTER(RKMomentEngine), POINTER(RKRadarDesc), POINTER(RKFFTModule), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKMomentEngineSetEssentials.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 96
if _libs["radarkit"].has("RKMomentEngineSetInputOutputBuffers", "cdecl"):
    RKMomentEngineSetInputOutputBuffers = _libs["radarkit"].get("RKMomentEngineSetInputOutputBuffers", "cdecl")
    RKMomentEngineSetInputOutputBuffers.argtypes = [POINTER(RKMomentEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKMomentEngineSetInputOutputBuffers.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 101
if _libs["radarkit"].has("RKMomentEngineSetFFTModule", "cdecl"):
    RKMomentEngineSetFFTModule = _libs["radarkit"].get("RKMomentEngineSetFFTModule", "cdecl")
    RKMomentEngineSetFFTModule.argtypes = [POINTER(RKMomentEngine), POINTER(RKFFTModule)]
    RKMomentEngineSetFFTModule.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 102
if _libs["radarkit"].has("RKMomentEngineSetCalibrator", "cdecl"):
    RKMomentEngineSetCalibrator = _libs["radarkit"].get("RKMomentEngineSetCalibrator", "cdecl")
    RKMomentEngineSetCalibrator.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(None), RKUserModule, POINTER(RKMomentScratch)), RKUserModule]
    RKMomentEngineSetCalibrator.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 103
if _libs["radarkit"].has("RKMomentEngineSetCoreCount", "cdecl"):
    RKMomentEngineSetCoreCount = _libs["radarkit"].get("RKMomentEngineSetCoreCount", "cdecl")
    RKMomentEngineSetCoreCount.argtypes = [POINTER(RKMomentEngine), uint8_t]
    RKMomentEngineSetCoreCount.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 104
if _libs["radarkit"].has("RKMomentEngineSetCoreOrigin", "cdecl"):
    RKMomentEngineSetCoreOrigin = _libs["radarkit"].get("RKMomentEngineSetCoreOrigin", "cdecl")
    RKMomentEngineSetCoreOrigin.argtypes = [POINTER(RKMomentEngine), uint8_t]
    RKMomentEngineSetCoreOrigin.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 105
if _libs["radarkit"].has("RKMomentEngineSetExcludeBoundaryPulses", "cdecl"):
    RKMomentEngineSetExcludeBoundaryPulses = _libs["radarkit"].get("RKMomentEngineSetExcludeBoundaryPulses", "cdecl")
    RKMomentEngineSetExcludeBoundaryPulses.argtypes = [POINTER(RKMomentEngine), c_bool]
    RKMomentEngineSetExcludeBoundaryPulses.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 106
if _libs["radarkit"].has("RKMomentEngineSetNoiseEstimator", "cdecl"):
    RKMomentEngineSetNoiseEstimator = _libs["radarkit"].get("RKMomentEngineSetNoiseEstimator", "cdecl")
    RKMomentEngineSetNoiseEstimator.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)]
    RKMomentEngineSetNoiseEstimator.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 107
if _libs["radarkit"].has("RKMomentEngineSetMomentProcessor", "cdecl"):
    RKMomentEngineSetMomentProcessor = _libs["radarkit"].get("RKMomentEngineSetMomentProcessor", "cdecl")
    RKMomentEngineSetMomentProcessor.argtypes = [POINTER(RKMomentEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t)]
    RKMomentEngineSetMomentProcessor.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 109
if _libs["radarkit"].has("RKMomentEngineStart", "cdecl"):
    RKMomentEngineStart = _libs["radarkit"].get("RKMomentEngineStart", "cdecl")
    RKMomentEngineStart.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineStart.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 110
if _libs["radarkit"].has("RKMomentEngineStop", "cdecl"):
    RKMomentEngineStop = _libs["radarkit"].get("RKMomentEngineStop", "cdecl")
    RKMomentEngineStop.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineStop.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 112
if _libs["radarkit"].has("RKMomentEngineGetProcessedRay", "cdecl"):
    RKMomentEngineGetProcessedRay = _libs["radarkit"].get("RKMomentEngineGetProcessedRay", "cdecl")
    RKMomentEngineGetProcessedRay.argtypes = [POINTER(RKMomentEngine), c_bool]
    RKMomentEngineGetProcessedRay.restype = POINTER(RKRay)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 113
if _libs["radarkit"].has("RKMomentEngineFlush", "cdecl"):
    RKMomentEngineFlush = _libs["radarkit"].get("RKMomentEngineFlush", "cdecl")
    RKMomentEngineFlush.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineFlush.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 114
if _libs["radarkit"].has("RKMomentEngineWaitWhileBusy", "cdecl"):
    RKMomentEngineWaitWhileBusy = _libs["radarkit"].get("RKMomentEngineWaitWhileBusy", "cdecl")
    RKMomentEngineWaitWhileBusy.argtypes = [POINTER(RKMomentEngine)]
    RKMomentEngineWaitWhileBusy.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 116
if _libs["radarkit"].has("RKMomentEngineStatusString", "cdecl"):
    RKMomentEngineStatusString = _libs["radarkit"].get("RKMomentEngineStatusString", "cdecl")
    RKMomentEngineStatusString.argtypes = [POINTER(RKMomentEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKMomentEngineStatusString.restype = ReturnString
    else:
        RKMomentEngineStatusString.restype = String
        RKMomentEngineStatusString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 118
if _libs["radarkit"].has("RKNoiseFromConfig", "cdecl"):
    RKNoiseFromConfig = _libs["radarkit"].get("RKNoiseFromConfig", "cdecl")
    RKNoiseFromConfig.argtypes = [POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t]
    RKNoiseFromConfig.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 26
class struct_rk_sweep_scratch(Structure):
    pass

struct_rk_sweep_scratch.__slots__ = [
    'filename',
    'filelist',
    'summary',
    'rays',
    'rayCount',
]
struct_rk_sweep_scratch._fields_ = [
    ('filename', c_char * int((1024 - 32))),
    ('filelist', c_char * int(4096)),
    ('summary', c_char * int(4096)),
    ('rays', POINTER(RKRay) * int(1500)),
    ('rayCount', uint32_t),
]

RKSweepScratchSpace = struct_rk_sweep_scratch# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 26

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 30
class struct_rk_sweep_engine(Structure):
    pass

RKSweepEngine = struct_rk_sweep_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 28

struct_rk_sweep_engine.__slots__ = [
    'name',
    'radarDescription',
    'rayBuffer',
    'rayIndex',
    'configBuffer',
    'configIndex',
    'verbose',
    'record',
    'convertToDegrees',
    'hasFileHandlingScript',
    'fileHandlingScript',
    'fileHandlingScriptProperties',
    'fileManager',
    'productTimeoutSeconds',
    'productFileExtension',
    'productRecorder',
    'sweepIndex',
    'tidRayGatherer',
    'scratchSpaces',
    'scratchSpaceIndex',
    'productBuffer',
    'productBufferDepth',
    'productIndex',
    'productMutex',
    'baseProductList',
    'baseMomentProductIds',
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
    ('convertToDegrees', c_bool),
    ('hasFileHandlingScript', c_bool),
    ('fileHandlingScript', c_char * int(1024)),
    ('fileHandlingScriptProperties', RKScriptProperty),
    ('fileManager', POINTER(RKFileManager)),
    ('productTimeoutSeconds', uint32_t),
    ('productFileExtension', c_char * int(8)),
    ('productRecorder', CFUNCTYPE(UNCHECKED(c_int), POINTER(RKProduct), String)),
    ('sweepIndex', RKIdentifier),
    ('tidRayGatherer', pthread_t),
    ('scratchSpaces', RKSweepScratchSpace * int(4)),
    ('scratchSpaceIndex', uint8_t),
    ('productBuffer', POINTER(RKProduct)),
    ('productBufferDepth', uint32_t),
    ('productIndex', uint32_t),
    ('productMutex', pthread_mutex_t),
    ('baseProductList', RKBaseProductList),
    ('baseMomentProductIds', RKProductId * int(RKBaseProductIndexCount)),
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 73
if _libs["radarkit"].has("RKSweepEngineInit", "cdecl"):
    RKSweepEngineInit = _libs["radarkit"].get("RKSweepEngineInit", "cdecl")
    RKSweepEngineInit.argtypes = []
    RKSweepEngineInit.restype = POINTER(RKSweepEngine)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 74
if _libs["radarkit"].has("RKSweepEngineFree", "cdecl"):
    RKSweepEngineFree = _libs["radarkit"].get("RKSweepEngineFree", "cdecl")
    RKSweepEngineFree.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 76
if _libs["radarkit"].has("RKSweepEngineSetVerbose", "cdecl"):
    RKSweepEngineSetVerbose = _libs["radarkit"].get("RKSweepEngineSetVerbose", "cdecl")
    RKSweepEngineSetVerbose.argtypes = [POINTER(RKSweepEngine), c_int]
    RKSweepEngineSetVerbose.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 77
if _libs["radarkit"].has("RKSweepEngineSetEssentials", "cdecl"):
    RKSweepEngineSetEssentials = _libs["radarkit"].get("RKSweepEngineSetEssentials", "cdecl")
    RKSweepEngineSetEssentials.argtypes = [POINTER(RKSweepEngine), POINTER(RKRadarDesc), POINTER(RKFileManager), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKSweepEngineSetEssentials.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 80
if _libs["radarkit"].has("RKSweepEngineSetRecord", "cdecl"):
    RKSweepEngineSetRecord = _libs["radarkit"].get("RKSweepEngineSetRecord", "cdecl")
    RKSweepEngineSetRecord.argtypes = [POINTER(RKSweepEngine), c_bool]
    RKSweepEngineSetRecord.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 81
for _lib in _libs.values():
    if not _lib.has("RKSweepEngineSetProductTimeout", "cdecl"):
        continue
    RKSweepEngineSetProductTimeout = _lib.get("RKSweepEngineSetProductTimeout", "cdecl")
    RKSweepEngineSetProductTimeout.argtypes = [POINTER(RKSweepEngine), uint32_t]
    RKSweepEngineSetProductTimeout.restype = None
    break

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 82
if _libs["radarkit"].has("RKSweepEngineSetFilesHandlingScript", "cdecl"):
    RKSweepEngineSetFilesHandlingScript = _libs["radarkit"].get("RKSweepEngineSetFilesHandlingScript", "cdecl")
    RKSweepEngineSetFilesHandlingScript.argtypes = [POINTER(RKSweepEngine), String, RKScriptProperty]
    RKSweepEngineSetFilesHandlingScript.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 83
if _libs["radarkit"].has("RKSweepEngineSetProductRecorder", "cdecl"):
    RKSweepEngineSetProductRecorder = _libs["radarkit"].get("RKSweepEngineSetProductRecorder", "cdecl")
    RKSweepEngineSetProductRecorder.argtypes = [POINTER(RKSweepEngine), CFUNCTYPE(UNCHECKED(c_int), POINTER(RKProduct), String)]
    RKSweepEngineSetProductRecorder.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 85
if _libs["radarkit"].has("RKSweepEngineStart", "cdecl"):
    RKSweepEngineStart = _libs["radarkit"].get("RKSweepEngineStart", "cdecl")
    RKSweepEngineStart.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineStart.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 86
if _libs["radarkit"].has("RKSweepEngineStop", "cdecl"):
    RKSweepEngineStop = _libs["radarkit"].get("RKSweepEngineStop", "cdecl")
    RKSweepEngineStop.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineStop.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 87
if _libs["radarkit"].has("RKSweepEngineFlush", "cdecl"):
    RKSweepEngineFlush = _libs["radarkit"].get("RKSweepEngineFlush", "cdecl")
    RKSweepEngineFlush.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineFlush.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 89
if _libs["radarkit"].has("RKSweepEngineStatusString", "cdecl"):
    RKSweepEngineStatusString = _libs["radarkit"].get("RKSweepEngineStatusString", "cdecl")
    RKSweepEngineStatusString.argtypes = [POINTER(RKSweepEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSweepEngineStatusString.restype = ReturnString
    else:
        RKSweepEngineStatusString.restype = String
        RKSweepEngineStatusString.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 90
if _libs["radarkit"].has("RKSweepEngineLatestSummary", "cdecl"):
    RKSweepEngineLatestSummary = _libs["radarkit"].get("RKSweepEngineLatestSummary", "cdecl")
    RKSweepEngineLatestSummary.argtypes = [POINTER(RKSweepEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKSweepEngineLatestSummary.restype = ReturnString
    else:
        RKSweepEngineLatestSummary.restype = String
        RKSweepEngineLatestSummary.errcheck = ReturnString

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 92
if _libs["radarkit"].has("RKSweepEngineDescribeProduct", "cdecl"):
    RKSweepEngineDescribeProduct = _libs["radarkit"].get("RKSweepEngineDescribeProduct", "cdecl")
    RKSweepEngineDescribeProduct.argtypes = [POINTER(RKSweepEngine), RKProductDesc]
    RKSweepEngineDescribeProduct.restype = RKProductId

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 93
if _libs["radarkit"].has("RKSweepEngineUndescribeProduct", "cdecl"):
    RKSweepEngineUndescribeProduct = _libs["radarkit"].get("RKSweepEngineUndescribeProduct", "cdecl")
    RKSweepEngineUndescribeProduct.argtypes = [POINTER(RKSweepEngine), RKProductId]
    RKSweepEngineUndescribeProduct.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 94
if _libs["radarkit"].has("RKSweepEngineGetVacantProduct", "cdecl"):
    RKSweepEngineGetVacantProduct = _libs["radarkit"].get("RKSweepEngineGetVacantProduct", "cdecl")
    RKSweepEngineGetVacantProduct.argtypes = [POINTER(RKSweepEngine), POINTER(RKSweep), RKProductId]
    RKSweepEngineGetVacantProduct.restype = POINTER(RKProduct)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 95
if _libs["radarkit"].has("RKSweepEngineSetProductComplete", "cdecl"):
    RKSweepEngineSetProductComplete = _libs["radarkit"].get("RKSweepEngineSetProductComplete", "cdecl")
    RKSweepEngineSetProductComplete.argtypes = [POINTER(RKSweepEngine), POINTER(RKSweep), POINTER(RKProduct)]
    RKSweepEngineSetProductComplete.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 96
if _libs["radarkit"].has("RKSweepEngineWaitWhileBusy", "cdecl"):
    RKSweepEngineWaitWhileBusy = _libs["radarkit"].get("RKSweepEngineWaitWhileBusy", "cdecl")
    RKSweepEngineWaitWhileBusy.argtypes = [POINTER(RKSweepEngine)]
    RKSweepEngineWaitWhileBusy.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 98
if _libs["radarkit"].has("RKSweepCollect", "cdecl"):
    RKSweepCollect = _libs["radarkit"].get("RKSweepCollect", "cdecl")
    RKSweepCollect.argtypes = [POINTER(RKSweepEngine), uint8_t]
    RKSweepCollect.restype = POINTER(RKSweep)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 99
if _libs["radarkit"].has("RKSweepFree", "cdecl"):
    RKSweepFree = _libs["radarkit"].get("RKSweepFree", "cdecl")
    RKSweepFree.argtypes = [POINTER(RKSweep)]
    RKSweepFree.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 21
class struct_rk_pulse_ring_filter_worker(Structure):
    pass

RKPulseRingFilterWorker = struct_rk_pulse_ring_filter_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 18

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 39
class struct_rk_pulse_ring_filter_engine(Structure):
    pass

RKPulseRingFilterEngine = struct_rk_pulse_ring_filter_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 19

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
    ('almostFull', c_int),
    ('memoryUsage', c_size_t),
]

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 71
if _libs["radarkit"].has("RKPulseRingFilterEngineInit", "cdecl"):
    RKPulseRingFilterEngineInit = _libs["radarkit"].get("RKPulseRingFilterEngineInit", "cdecl")
    RKPulseRingFilterEngineInit.argtypes = []
    RKPulseRingFilterEngineInit.restype = POINTER(RKPulseRingFilterEngine)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 72
if _libs["radarkit"].has("RKPulseRingFilterEngineFree", "cdecl"):
    RKPulseRingFilterEngineFree = _libs["radarkit"].get("RKPulseRingFilterEngineFree", "cdecl")
    RKPulseRingFilterEngineFree.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineFree.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 74
if _libs["radarkit"].has("RKPulseRingFilterEngineSetVerbose", "cdecl"):
    RKPulseRingFilterEngineSetVerbose = _libs["radarkit"].get("RKPulseRingFilterEngineSetVerbose", "cdecl")
    RKPulseRingFilterEngineSetVerbose.argtypes = [POINTER(RKPulseRingFilterEngine), c_int]
    RKPulseRingFilterEngineSetVerbose.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 75
if _libs["radarkit"].has("RKPulseRingFilterEngineSetEssentials", "cdecl"):
    RKPulseRingFilterEngineSetEssentials = _libs["radarkit"].get("RKPulseRingFilterEngineSetEssentials", "cdecl")
    RKPulseRingFilterEngineSetEssentials.argtypes = [POINTER(RKPulseRingFilterEngine), POINTER(RKRadarDesc), POINTER(RKConfig), POINTER(uint32_t), RKBuffer, POINTER(uint32_t)]
    RKPulseRingFilterEngineSetEssentials.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 78
if _libs["radarkit"].has("RKPulseRingFilterEngineSetCoreCount", "cdecl"):
    RKPulseRingFilterEngineSetCoreCount = _libs["radarkit"].get("RKPulseRingFilterEngineSetCoreCount", "cdecl")
    RKPulseRingFilterEngineSetCoreCount.argtypes = [POINTER(RKPulseRingFilterEngine), uint8_t]
    RKPulseRingFilterEngineSetCoreCount.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 79
if _libs["radarkit"].has("RKPulseRingFilterEngineSetCoreOrigin", "cdecl"):
    RKPulseRingFilterEngineSetCoreOrigin = _libs["radarkit"].get("RKPulseRingFilterEngineSetCoreOrigin", "cdecl")
    RKPulseRingFilterEngineSetCoreOrigin.argtypes = [POINTER(RKPulseRingFilterEngine), uint8_t]
    RKPulseRingFilterEngineSetCoreOrigin.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 81
if _libs["radarkit"].has("RKPulseRingFilterEngineEnableFilter", "cdecl"):
    RKPulseRingFilterEngineEnableFilter = _libs["radarkit"].get("RKPulseRingFilterEngineEnableFilter", "cdecl")
    RKPulseRingFilterEngineEnableFilter.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineEnableFilter.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 82
if _libs["radarkit"].has("RKPulseRingFilterEngineDisableFilter", "cdecl"):
    RKPulseRingFilterEngineDisableFilter = _libs["radarkit"].get("RKPulseRingFilterEngineDisableFilter", "cdecl")
    RKPulseRingFilterEngineDisableFilter.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineDisableFilter.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 83
if _libs["radarkit"].has("RKPulseRingFilterEngineSetFilter", "cdecl"):
    RKPulseRingFilterEngineSetFilter = _libs["radarkit"].get("RKPulseRingFilterEngineSetFilter", "cdecl")
    RKPulseRingFilterEngineSetFilter.argtypes = [POINTER(RKPulseRingFilterEngine), POINTER(RKIIRFilter)]
    RKPulseRingFilterEngineSetFilter.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 85
if _libs["radarkit"].has("RKPulseRingFilterEngineStart", "cdecl"):
    RKPulseRingFilterEngineStart = _libs["radarkit"].get("RKPulseRingFilterEngineStart", "cdecl")
    RKPulseRingFilterEngineStart.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineStart.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 86
if _libs["radarkit"].has("RKPulseRingFilterEngineStop", "cdecl"):
    RKPulseRingFilterEngineStop = _libs["radarkit"].get("RKPulseRingFilterEngineStop", "cdecl")
    RKPulseRingFilterEngineStop.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineStop.restype = c_int

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 88
if _libs["radarkit"].has("RKPulseRingFilterEngineShowFilterSummary", "cdecl"):
    RKPulseRingFilterEngineShowFilterSummary = _libs["radarkit"].get("RKPulseRingFilterEngineShowFilterSummary", "cdecl")
    RKPulseRingFilterEngineShowFilterSummary.argtypes = [POINTER(RKPulseRingFilterEngine)]
    RKPulseRingFilterEngineShowFilterSummary.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 89
if _libs["radarkit"].has("RKPulseRingFilterEngineStatusString", "cdecl"):
    RKPulseRingFilterEngineStatusString = _libs["radarkit"].get("RKPulseRingFilterEngineStatusString", "cdecl")
    RKPulseRingFilterEngineStatusString.argtypes = [POINTER(RKPulseRingFilterEngine)]
    if sizeof(c_int) == sizeof(c_void_p):
        RKPulseRingFilterEngineStatusString.restype = ReturnString
    else:
        RKPulseRingFilterEngineStatusString.restype = String
        RKPulseRingFilterEngineStatusString.errcheck = ReturnString

RKNetworkSocketType = c_int# headers/RadarKit/RKNetwork.h: 21

RKNetworkMessageFormat = c_int# headers/RadarKit/RKNetwork.h: 27

# headers/RadarKit/RKNetwork.h: 57
class struct_anon_169(Structure):
    pass

struct_anon_169._pack_ = 1
struct_anon_169.__slots__ = [
    'type',
    'subtype',
    'size',
    'decodedSize',
]
struct_anon_169._fields_ = [
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
    ('unnamed_1', struct_anon_169),
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
    ('actionIndex', c_int),
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

RKTestFlag = uint8_t# headers/RadarKit/RKTest.h: 17

enum_anon_208 = c_int# headers/RadarKit/RKTest.h: 18

RKTestFlagNone = 0# headers/RadarKit/RKTest.h: 18

RKTestFlagVerbose = 1# headers/RadarKit/RKTest.h: 18

RKTestFlagShowResults = (1 << 1)# headers/RadarKit/RKTest.h: 18

RKTestSIMDFlag = uint8_t# headers/RadarKit/RKTest.h: 24

enum_anon_209 = c_int# headers/RadarKit/RKTest.h: 25

RKTestSIMDFlagNull = 0# headers/RadarKit/RKTest.h: 25

RKTestSIMDFlagShowNumbers = 1# headers/RadarKit/RKTest.h: 25

RKTestSIMDFlagPerformanceTestArithmetic = (1 << 1)# headers/RadarKit/RKTest.h: 25

RKTestSIMDFlagPerformanceTestDuplicate = (1 << 2)# headers/RadarKit/RKTest.h: 25

RKTestSIMDFlagPerformanceTestAll = (RKTestSIMDFlagPerformanceTestArithmetic | RKTestSIMDFlagPerformanceTestDuplicate)# headers/RadarKit/RKTest.h: 25

RKAxisAction = uint8_t# headers/RadarKit/RKTest.h: 33

enum_anon_210 = c_int# headers/RadarKit/RKTest.h: 34

RKAxisActionStop = 0# headers/RadarKit/RKTest.h: 34

RKAxisActionSpeed = (RKAxisActionStop + 1)# headers/RadarKit/RKTest.h: 34

RKAxisActionPosition = (RKAxisActionSpeed + 1)# headers/RadarKit/RKTest.h: 34

# headers/RadarKit/RKTest.h: 70
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

RKTestTransceiver = struct_rk_test_transceiver# headers/RadarKit/RKTest.h: 70

# headers/RadarKit/RKTest.h: 94
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

RKTestPedestal = struct_rk_test_pedestal# headers/RadarKit/RKTest.h: 94

# headers/RadarKit/RKTest.h: 106
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

RKTestHealthRelay = struct_rk_test_health_relay# headers/RadarKit/RKTest.h: 106

# headers/RadarKit/RKTest.h: 110
if _libs["radarkit"].has("RKTestByNumberDescription", "cdecl"):
    RKTestByNumberDescription = _libs["radarkit"].get("RKTestByNumberDescription", "cdecl")
    RKTestByNumberDescription.argtypes = [c_int]
    if sizeof(c_int) == sizeof(c_void_p):
        RKTestByNumberDescription.restype = ReturnString
    else:
        RKTestByNumberDescription.restype = String
        RKTestByNumberDescription.errcheck = ReturnString

# headers/RadarKit/RKTest.h: 111
if _libs["radarkit"].has("RKTestByNumber", "cdecl"):
    RKTestByNumber = _libs["radarkit"].get("RKTestByNumber", "cdecl")
    RKTestByNumber.argtypes = [c_int, POINTER(None)]
    RKTestByNumber.restype = None

# headers/RadarKit/RKTest.h: 115
if _libs["radarkit"].has("RKTestTerminalColors", "cdecl"):
    RKTestTerminalColors = _libs["radarkit"].get("RKTestTerminalColors", "cdecl")
    RKTestTerminalColors.argtypes = []
    RKTestTerminalColors.restype = None

# headers/RadarKit/RKTest.h: 116
if _libs["radarkit"].has("RKTestPrettyStrings", "cdecl"):
    RKTestPrettyStrings = _libs["radarkit"].get("RKTestPrettyStrings", "cdecl")
    RKTestPrettyStrings.argtypes = []
    RKTestPrettyStrings.restype = None

# headers/RadarKit/RKTest.h: 117
if _libs["radarkit"].has("RKTestBasicMath", "cdecl"):
    RKTestBasicMath = _libs["radarkit"].get("RKTestBasicMath", "cdecl")
    RKTestBasicMath.argtypes = []
    RKTestBasicMath.restype = None

# headers/RadarKit/RKTest.h: 118
if _libs["radarkit"].has("RKTestParseCommaDelimitedValues", "cdecl"):
    RKTestParseCommaDelimitedValues = _libs["radarkit"].get("RKTestParseCommaDelimitedValues", "cdecl")
    RKTestParseCommaDelimitedValues.argtypes = []
    RKTestParseCommaDelimitedValues.restype = None

# headers/RadarKit/RKTest.h: 119
if _libs["radarkit"].has("RKTestParseJSONString", "cdecl"):
    RKTestParseJSONString = _libs["radarkit"].get("RKTestParseJSONString", "cdecl")
    RKTestParseJSONString.argtypes = []
    RKTestParseJSONString.restype = None

# headers/RadarKit/RKTest.h: 120
if _libs["radarkit"].has("RKTestFileManager", "cdecl"):
    RKTestFileManager = _libs["radarkit"].get("RKTestFileManager", "cdecl")
    RKTestFileManager.argtypes = []
    RKTestFileManager.restype = None

# headers/RadarKit/RKTest.h: 121
if _libs["radarkit"].has("RKTestPreferenceReading", "cdecl"):
    RKTestPreferenceReading = _libs["radarkit"].get("RKTestPreferenceReading", "cdecl")
    RKTestPreferenceReading.argtypes = []
    RKTestPreferenceReading.restype = None

# headers/RadarKit/RKTest.h: 122
if _libs["radarkit"].has("RKTestCountFiles", "cdecl"):
    RKTestCountFiles = _libs["radarkit"].get("RKTestCountFiles", "cdecl")
    RKTestCountFiles.argtypes = []
    RKTestCountFiles.restype = None

# headers/RadarKit/RKTest.h: 123
if _libs["radarkit"].has("RKTestFileMonitor", "cdecl"):
    RKTestFileMonitor = _libs["radarkit"].get("RKTestFileMonitor", "cdecl")
    RKTestFileMonitor.argtypes = []
    RKTestFileMonitor.restype = None

# headers/RadarKit/RKTest.h: 124
if _libs["radarkit"].has("RKTestHostMonitor", "cdecl"):
    RKTestHostMonitor = _libs["radarkit"].get("RKTestHostMonitor", "cdecl")
    RKTestHostMonitor.argtypes = []
    RKTestHostMonitor.restype = None

# headers/RadarKit/RKTest.h: 125
if _libs["radarkit"].has("RKTestInitializingRadar", "cdecl"):
    RKTestInitializingRadar = _libs["radarkit"].get("RKTestInitializingRadar", "cdecl")
    RKTestInitializingRadar.argtypes = []
    RKTestInitializingRadar.restype = None

# headers/RadarKit/RKTest.h: 126
if _libs["radarkit"].has("RKTestTemperatureToStatus", "cdecl"):
    RKTestTemperatureToStatus = _libs["radarkit"].get("RKTestTemperatureToStatus", "cdecl")
    RKTestTemperatureToStatus.argtypes = []
    RKTestTemperatureToStatus.restype = None

# headers/RadarKit/RKTest.h: 127
if _libs["radarkit"].has("RKTestGetCountry", "cdecl"):
    RKTestGetCountry = _libs["radarkit"].get("RKTestGetCountry", "cdecl")
    RKTestGetCountry.argtypes = []
    RKTestGetCountry.restype = None

# headers/RadarKit/RKTest.h: 128
if _libs["radarkit"].has("RKTestBufferOverviewText", "cdecl"):
    RKTestBufferOverviewText = _libs["radarkit"].get("RKTestBufferOverviewText", "cdecl")
    RKTestBufferOverviewText.argtypes = [String]
    RKTestBufferOverviewText.restype = None

# headers/RadarKit/RKTest.h: 129
if _libs["radarkit"].has("RKTestHealthOverviewText", "cdecl"):
    RKTestHealthOverviewText = _libs["radarkit"].get("RKTestHealthOverviewText", "cdecl")
    RKTestHealthOverviewText.argtypes = [String]
    RKTestHealthOverviewText.restype = None

# headers/RadarKit/RKTest.h: 130
if _libs["radarkit"].has("RKTestSweepRead", "cdecl"):
    RKTestSweepRead = _libs["radarkit"].get("RKTestSweepRead", "cdecl")
    RKTestSweepRead.argtypes = [String]
    RKTestSweepRead.restype = None

# headers/RadarKit/RKTest.h: 131
if _libs["radarkit"].has("RKTestProductRead", "cdecl"):
    RKTestProductRead = _libs["radarkit"].get("RKTestProductRead", "cdecl")
    RKTestProductRead.argtypes = [String]
    RKTestProductRead.restype = None

# headers/RadarKit/RKTest.h: 132
if _libs["radarkit"].has("RKTestProductWrite", "cdecl"):
    RKTestProductWrite = _libs["radarkit"].get("RKTestProductWrite", "cdecl")
    RKTestProductWrite.argtypes = []
    RKTestProductWrite.restype = None

# headers/RadarKit/RKTest.h: 133
if _libs["radarkit"].has("RKTestReviseLogicalValues", "cdecl"):
    RKTestReviseLogicalValues = _libs["radarkit"].get("RKTestReviseLogicalValues", "cdecl")
    RKTestReviseLogicalValues.argtypes = []
    RKTestReviseLogicalValues.restype = None

# headers/RadarKit/RKTest.h: 134
if _libs["radarkit"].has("RKTestReadIQ", "cdecl"):
    RKTestReadIQ = _libs["radarkit"].get("RKTestReadIQ", "cdecl")
    RKTestReadIQ.argtypes = [String]
    RKTestReadIQ.restype = None

# headers/RadarKit/RKTest.h: 135
if _libs["radarkit"].has("RKTestPreparePath", "cdecl"):
    RKTestPreparePath = _libs["radarkit"].get("RKTestPreparePath", "cdecl")
    RKTestPreparePath.argtypes = []
    RKTestPreparePath.restype = None

# headers/RadarKit/RKTest.h: 136
if _libs["radarkit"].has("RKTestWebSocket", "cdecl"):
    RKTestWebSocket = _libs["radarkit"].get("RKTestWebSocket", "cdecl")
    RKTestWebSocket.argtypes = []
    RKTestWebSocket.restype = None

# headers/RadarKit/RKTest.h: 137
if _libs["radarkit"].has("RKTestReadBareRKComplex", "cdecl"):
    RKTestReadBareRKComplex = _libs["radarkit"].get("RKTestReadBareRKComplex", "cdecl")
    RKTestReadBareRKComplex.argtypes = [String]
    RKTestReadBareRKComplex.restype = None

# headers/RadarKit/RKTest.h: 138
if _libs["radarkit"].has("RKTestRadarHub", "cdecl"):
    RKTestRadarHub = _libs["radarkit"].get("RKTestRadarHub", "cdecl")
    RKTestRadarHub.argtypes = []
    RKTestRadarHub.restype = None

# headers/RadarKit/RKTest.h: 139
if _libs["radarkit"].has("RKTestSimplePulseEngine", "cdecl"):
    RKTestSimplePulseEngine = _libs["radarkit"].get("RKTestSimplePulseEngine", "cdecl")
    RKTestSimplePulseEngine.argtypes = [RKPulseStatus]
    RKTestSimplePulseEngine.restype = None

# headers/RadarKit/RKTest.h: 140
if _libs["radarkit"].has("RKTestSimpleMomentEngine", "cdecl"):
    RKTestSimpleMomentEngine = _libs["radarkit"].get("RKTestSimpleMomentEngine", "cdecl")
    RKTestSimpleMomentEngine.argtypes = [c_int]
    RKTestSimpleMomentEngine.restype = None

# headers/RadarKit/RKTest.h: 144
if _libs["radarkit"].has("RKTestSIMD", "cdecl"):
    RKTestSIMD = _libs["radarkit"].get("RKTestSIMD", "cdecl")
    RKTestSIMD.argtypes = [RKTestSIMDFlag, c_int]
    RKTestSIMD.restype = None

# headers/RadarKit/RKTest.h: 145
if _libs["radarkit"].has("RKTestWindow", "cdecl"):
    RKTestWindow = _libs["radarkit"].get("RKTestWindow", "cdecl")
    RKTestWindow.argtypes = []
    RKTestWindow.restype = None

# headers/RadarKit/RKTest.h: 146
if _libs["radarkit"].has("RKTestHilbertTransform", "cdecl"):
    RKTestHilbertTransform = _libs["radarkit"].get("RKTestHilbertTransform", "cdecl")
    RKTestHilbertTransform.argtypes = []
    RKTestHilbertTransform.restype = None

# headers/RadarKit/RKTest.h: 147
if _libs["radarkit"].has("RKTestWriteFFTWisdom", "cdecl"):
    RKTestWriteFFTWisdom = _libs["radarkit"].get("RKTestWriteFFTWisdom", "cdecl")
    RKTestWriteFFTWisdom.argtypes = [c_int]
    RKTestWriteFFTWisdom.restype = None

# headers/RadarKit/RKTest.h: 148
if _libs["radarkit"].has("RKTestRingFilterShowCoefficients", "cdecl"):
    RKTestRingFilterShowCoefficients = _libs["radarkit"].get("RKTestRingFilterShowCoefficients", "cdecl")
    RKTestRingFilterShowCoefficients.argtypes = []
    RKTestRingFilterShowCoefficients.restype = None

# headers/RadarKit/RKTest.h: 149
if _libs["radarkit"].has("RKTestRamp", "cdecl"):
    RKTestRamp = _libs["radarkit"].get("RKTestRamp", "cdecl")
    RKTestRamp.argtypes = []
    RKTestRamp.restype = None

# headers/RadarKit/RKTest.h: 153
if _libs["radarkit"].has("RKTestMakeHops", "cdecl"):
    RKTestMakeHops = _libs["radarkit"].get("RKTestMakeHops", "cdecl")
    RKTestMakeHops.argtypes = []
    RKTestMakeHops.restype = None

# headers/RadarKit/RKTest.h: 154
if _libs["radarkit"].has("RKTestWaveformTFM", "cdecl"):
    RKTestWaveformTFM = _libs["radarkit"].get("RKTestWaveformTFM", "cdecl")
    RKTestWaveformTFM.argtypes = []
    RKTestWaveformTFM.restype = None

# headers/RadarKit/RKTest.h: 155
if _libs["radarkit"].has("RKTestWaveformWrite", "cdecl"):
    RKTestWaveformWrite = _libs["radarkit"].get("RKTestWaveformWrite", "cdecl")
    RKTestWaveformWrite.argtypes = []
    RKTestWaveformWrite.restype = None

# headers/RadarKit/RKTest.h: 156
if _libs["radarkit"].has("RKTestWaveformDownsampling", "cdecl"):
    RKTestWaveformDownsampling = _libs["radarkit"].get("RKTestWaveformDownsampling", "cdecl")
    RKTestWaveformDownsampling.argtypes = []
    RKTestWaveformDownsampling.restype = None

# headers/RadarKit/RKTest.h: 157
if _libs["radarkit"].has("RKTestWaveformShowProperties", "cdecl"):
    RKTestWaveformShowProperties = _libs["radarkit"].get("RKTestWaveformShowProperties", "cdecl")
    RKTestWaveformShowProperties.argtypes = []
    RKTestWaveformShowProperties.restype = None

# headers/RadarKit/RKTest.h: 158
if _libs["radarkit"].has("RKTestWaveformShowUserWaveformProperties", "cdecl"):
    RKTestWaveformShowUserWaveformProperties = _libs["radarkit"].get("RKTestWaveformShowUserWaveformProperties", "cdecl")
    RKTestWaveformShowUserWaveformProperties.argtypes = [String]
    RKTestWaveformShowUserWaveformProperties.restype = None

# headers/RadarKit/RKTest.h: 162
if _libs["radarkit"].has("RKTestPulseCompression", "cdecl"):
    RKTestPulseCompression = _libs["radarkit"].get("RKTestPulseCompression", "cdecl")
    RKTestPulseCompression.argtypes = [RKTestFlag]
    RKTestPulseCompression.restype = None

# headers/RadarKit/RKTest.h: 163
if _libs["radarkit"].has("RKTestOnePulse", "cdecl"):
    RKTestOnePulse = _libs["radarkit"].get("RKTestOnePulse", "cdecl")
    RKTestOnePulse.argtypes = []
    RKTestOnePulse.restype = None

# headers/RadarKit/RKTest.h: 164
if _libs["radarkit"].has("RKTestOneRay", "cdecl"):
    RKTestOneRay = _libs["radarkit"].get("RKTestOneRay", "cdecl")
    RKTestOneRay.argtypes = [CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t), c_int]
    RKTestOneRay.restype = None

# headers/RadarKit/RKTest.h: 165
if _libs["radarkit"].has("RKTestOneRaySpectra", "cdecl"):
    RKTestOneRaySpectra = _libs["radarkit"].get("RKTestOneRaySpectra", "cdecl")
    RKTestOneRaySpectra.argtypes = [CFUNCTYPE(UNCHECKED(c_int), POINTER(RKMomentScratch), POINTER(POINTER(RKPulse)), uint16_t), c_int]
    RKTestOneRaySpectra.restype = None

# headers/RadarKit/RKTest.h: 169
if _libs["radarkit"].has("RKTestPulseCompressionSpeed", "cdecl"):
    RKTestPulseCompressionSpeed = _libs["radarkit"].get("RKTestPulseCompressionSpeed", "cdecl")
    RKTestPulseCompressionSpeed.argtypes = [c_int]
    RKTestPulseCompressionSpeed.restype = None

# headers/RadarKit/RKTest.h: 170
if _libs["radarkit"].has("RKTestPulseEngineSpeed", "cdecl"):
    RKTestPulseEngineSpeed = _libs["radarkit"].get("RKTestPulseEngineSpeed", "cdecl")
    RKTestPulseEngineSpeed.argtypes = [c_int]
    RKTestPulseEngineSpeed.restype = None

# headers/RadarKit/RKTest.h: 171
if _libs["radarkit"].has("RKTestMomentProcessorSpeed", "cdecl"):
    RKTestMomentProcessorSpeed = _libs["radarkit"].get("RKTestMomentProcessorSpeed", "cdecl")
    RKTestMomentProcessorSpeed.argtypes = []
    RKTestMomentProcessorSpeed.restype = None

# headers/RadarKit/RKTest.h: 172
if _libs["radarkit"].has("RKTestCacheWrite", "cdecl"):
    RKTestCacheWrite = _libs["radarkit"].get("RKTestCacheWrite", "cdecl")
    RKTestCacheWrite.argtypes = []
    RKTestCacheWrite.restype = None

# headers/RadarKit/RKTest.h: 176
if _libs["radarkit"].has("RKTestTransceiverInit", "cdecl"):
    RKTestTransceiverInit = _libs["radarkit"].get("RKTestTransceiverInit", "cdecl")
    RKTestTransceiverInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestTransceiverInit.restype = RKTransceiver

# headers/RadarKit/RKTest.h: 177
if _libs["radarkit"].has("RKTestTransceiverExec", "cdecl"):
    RKTestTransceiverExec = _libs["radarkit"].get("RKTestTransceiverExec", "cdecl")
    RKTestTransceiverExec.argtypes = [RKTransceiver, String, String]
    RKTestTransceiverExec.restype = c_int

# headers/RadarKit/RKTest.h: 178
if _libs["radarkit"].has("RKTestTransceiverFree", "cdecl"):
    RKTestTransceiverFree = _libs["radarkit"].get("RKTestTransceiverFree", "cdecl")
    RKTestTransceiverFree.argtypes = [RKTransceiver]
    RKTestTransceiverFree.restype = c_int

# headers/RadarKit/RKTest.h: 182
if _libs["radarkit"].has("RKTestPedestalInit", "cdecl"):
    RKTestPedestalInit = _libs["radarkit"].get("RKTestPedestalInit", "cdecl")
    RKTestPedestalInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestPedestalInit.restype = RKPedestal

# headers/RadarKit/RKTest.h: 183
if _libs["radarkit"].has("RKTestPedestalExec", "cdecl"):
    RKTestPedestalExec = _libs["radarkit"].get("RKTestPedestalExec", "cdecl")
    RKTestPedestalExec.argtypes = [RKPedestal, String, String]
    RKTestPedestalExec.restype = c_int

# headers/RadarKit/RKTest.h: 184
if _libs["radarkit"].has("RKTestPedestalFree", "cdecl"):
    RKTestPedestalFree = _libs["radarkit"].get("RKTestPedestalFree", "cdecl")
    RKTestPedestalFree.argtypes = [RKPedestal]
    RKTestPedestalFree.restype = c_int

# headers/RadarKit/RKTest.h: 188
if _libs["radarkit"].has("RKTestHealthRelayInit", "cdecl"):
    RKTestHealthRelayInit = _libs["radarkit"].get("RKTestHealthRelayInit", "cdecl")
    RKTestHealthRelayInit.argtypes = [POINTER(RKRadar), POINTER(None)]
    RKTestHealthRelayInit.restype = RKHealthRelay

# headers/RadarKit/RKTest.h: 189
if _libs["radarkit"].has("RKTestHealthRelayExec", "cdecl"):
    RKTestHealthRelayExec = _libs["radarkit"].get("RKTestHealthRelayExec", "cdecl")
    RKTestHealthRelayExec.argtypes = [RKHealthRelay, String, String]
    RKTestHealthRelayExec.restype = c_int

# headers/RadarKit/RKTest.h: 190
if _libs["radarkit"].has("RKTestHealthRelayFree", "cdecl"):
    RKTestHealthRelayFree = _libs["radarkit"].get("RKTestHealthRelayFree", "cdecl")
    RKTestHealthRelayFree.argtypes = [RKHealthRelay]
    RKTestHealthRelayFree.restype = c_int

# headers/RadarKit/RKTest.h: 194
if _libs["radarkit"].has("RKTestCommandQueue", "cdecl"):
    RKTestCommandQueue = _libs["radarkit"].get("RKTestCommandQueue", "cdecl")
    RKTestCommandQueue.argtypes = []
    RKTestCommandQueue.restype = None

# headers/RadarKit/RKTest.h: 195
if _libs["radarkit"].has("RKTestSingleCommand", "cdecl"):
    RKTestSingleCommand = _libs["radarkit"].get("RKTestSingleCommand", "cdecl")
    RKTestSingleCommand.argtypes = []
    RKTestSingleCommand.restype = None

# headers/RadarKit/RKTest.h: 196
if _libs["radarkit"].has("RKTestExperiment", "cdecl"):
    RKTestExperiment = _libs["radarkit"].get("RKTestExperiment", "cdecl")
    RKTestExperiment.argtypes = []
    RKTestExperiment.restype = None

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 11
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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 60
try:
    RKRawDataVersion = 7
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 61
try:
    RKBufferSSlotCount = 10
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 62
try:
    RKBufferCSlotCount = 10
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 63
try:
    RKBufferHSlotCount = 50
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 64
try:
    RKBufferPSlotCount = 1000
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 65
try:
    RKBuffer0SlotCount = 20000
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 66
try:
    RKBuffer2SlotCount = 3600
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 67
try:
    RKBuffer3SlotCount = 100
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 68
try:
    RKMaximumControlCount = 128
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 69
try:
    RKMaximumWaveformCalibrationCount = 128
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 70
try:
    RKMaximumGateCount = 262144
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 71
try:
    RKMemoryAlignSize = 64
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 72
try:
    RKMomentCount = 26
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 73
try:
    RKBaseProductCount = 16
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 74
try:
    RKMaximumLagCount = 5
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 75
try:
    RKMaximumFilterCount = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 76
try:
    RKMaximumWaveformCount = 22
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 77
try:
    RKWorkerDutyCycleBufferDepth = 1000
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 78
try:
    RKMaximumPulsesPerRay = 2000
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 79
try:
    RKMaximumRaysPerSweep = 1500
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 80
try:
    RKMaximumPacketSize = ((16 * 1024) * 1024)
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 81
try:
    RKNetworkTimeoutSeconds = 20
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 82
try:
    RKNetworkReconnectSeconds = 3
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 83
try:
    RKLagRedThreshold = 0.5
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 84
try:
    RKLagOrangeThreshold = 0.7
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 85
try:
    RKDutyCyleRedThreshold = 0.9
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 86
try:
    RKDutyCyleOrangeThreshold = 0.8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 87
try:
    RKStatusBarWidth = 6
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 88
try:
    RKPulseCountForNoiseMeasurement = 200
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 89
try:
    RKProcessorStatusPulseCoreCount = 16
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 90
try:
    RKProcessorStatusRingCoreCount = 16
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 91
try:
    RKProcessorStatusRayCoreCount = 16
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 92
try:
    RKHostMonitorPingInterval = 5
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 93
try:
    RKMaximumProductCount = 64
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 94
try:
    RKMaximumIIRFilterTaps = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 95
try:
    RKMaximumPrefixLength = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 96
try:
    RKMaximumSymbolLength = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 97
try:
    RKMaximumFileExtensionLength = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 98
try:
    RKUserParameterCount = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 99
try:
    RKMaximumScanCount = 256
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 100
try:
    RKPedestalActionBufferDepth = 8
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 101
try:
    RKDefaultScanSpeed = 18.0
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 103
try:
    RKDefaultDataPath = 'data'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 104
try:
    RKDataFolderIQ = 'iq'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 105
try:
    RKDataFolderMoment = 'moment'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 106
try:
    RKDataFolderHealth = 'health'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 107
try:
    RKLogFolder = 'log'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 108
try:
    RKWaveformFolder = 'waveforms'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 109
try:
    RKFFTWisdomFile = 'radarkit-fft-wisdom'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 111
try:
    RKNoColor = '\\033[m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 112
try:
    RKNoForegroundColor = '\\033[39m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 113
try:
    RKNoBackgroundColor = '\\033[49m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 114
try:
    RKBaseRedColor = '\\033[91m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 115
try:
    RKBaseGreenColor = '\\033[92m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 116
try:
    RKBaseYellowColor = '\\033[93m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 117
try:
    RKRedColor = '\\033[38;5;196m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 118
try:
    RKOrangeColor = '\\033[38;5;208m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 119
try:
    RKYellowColor = '\\033[38;5;226m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 120
try:
    RKCreamColor = '\\033[38;5;229m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 121
try:
    RKLimeColor = '\\033[38;5;118m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 122
try:
    RKMintColor = '\\033[38;5;43m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 123
try:
    RKGreenColor = '\\033[38;5;46m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 124
try:
    RKTealColor = '\\033[38;5;49m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 125
try:
    RKIceBlueColor = '\\033[38;5;51m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 126
try:
    RKSkyBlueColor = '\\033[38;5;45m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 127
try:
    RKBlueColor = '\\033[38;5;27m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 128
try:
    RKPurpleColor = '\\033[38;5;99m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 129
try:
    RKIndigoColor = '\\033[38;5;201m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 130
try:
    RKHotPinkColor = '\\033[38;5;199m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 131
try:
    RKDeepPinkColor = '\\033[38;5;197m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 132
try:
    RKPinkColor = '\\033[38;5;213m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 133
try:
    RKSalmonColor = '\\033[38;5;210m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 134
try:
    RKGrayColor = '\\033[38;5;245m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 135
try:
    RKWhiteColor = '\\033[38;5;15m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 136
try:
    RKMonokaiRed = '\\033[38;5;196m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 137
try:
    RKMonokaiPink = '\\033[38;5;197m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 138
try:
    RKMonokaiOrange = '\\033[38;5;208m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 139
try:
    RKMonokaiYellow = '\\033[38;5;186m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 140
try:
    RKMonokaiGreen = '\\033[38;5;154m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 141
try:
    RKMonokaiBlue = '\\033[38;5;81m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 142
try:
    RKMonokaiPurple = '\\033[38;5;141m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 143
try:
    RKLightOrangeColor = '\\033[38;5;220m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 144
try:
    RKWarningColor = '\\033[38;5;15;48;5;197m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 145
try:
    RKPythonColor = '\\033[38;5;226;48;5;24m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 146
try:
    RKRadarKitColor = '\\033[38;5;15;48;5;124m'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 147
try:
    RKMaximumStringLength = 4096
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 148
try:
    RKMaximumPathLength = 1024
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 149
try:
    RKMaximumFolderPathLength = 768
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 150
try:
    RKMaximumCommandLength = 512
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 151
try:
    RKStatusStringLength = 256
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 152
try:
    RKPulseHeaderPaddedSize = 384
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 153
try:
    RKRayHeaderPaddedSize = 128
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 154
try:
    RKNameLength = 128
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 155
try:
    RKShortNameLength = 20
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 156
try:
    RKChildNameLength = 160
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 158
def RKColorDutyCycle(x):
    return (x > RKDutyCyleRedThreshold) and RKBaseRedColor or (x > RKDutyCyleOrangeThreshold) and RKBaseYellowColor or RKBaseGreenColor

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 159
def RKColorLag(x):
    return (x > RKLagRedThreshold) and RKBaseRedColor or (x > RKLagOrangeThreshold) and RKBaseYellowColor or RKBaseGreenColor

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 161
def RKDigitWidth(v, n):
    return (c_int (ord_if_char((((floorf ((log10f ((fabsf (v)))))) + (v < 0)) + n and (n + 2) or 1)))).value

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 163
def ITALIC(x):
    return (('\\033[3m' + x) + '\\033[23m')

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 164
def UNDERLINE(x):
    return (('\\033[4m' + x) + '\\033[24m')

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 165
def HIGHLIGHT(x):
    return (('\\033[38;5;82;48;5;238m' + x) + '\\033[m')

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 166
def UNDERLINE_ITALIC(x):
    return (('\\033[3;4m' + x) + '\\033[23;24m')

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 168
def CLAMP(x, lo, hi):
    return (MIN ((MAX (x, lo)), hi))

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 174
def RKMarkerScanTypeString(x):
    return ((x & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) and 'PPI' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) and 'RHI' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint) and 'SPT' or 'UNK'

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 179
def RKMarkerScanTypeShortString(x):
    return ((x & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) and 'P' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) and 'R' or ((x & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint) and 'S' or 'U'

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 184
def RKPositionAzimuthFlagColor(x):
    return (x & RKPositionFlagAzimuthError) and RKRedColor or (x & RKPositionFlagAzimuthEnabled) and RKGreenColor or RKYellowColor

# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 188
def RKPositionElevationFlagColor(x):
    return (x & RKPositionFlagElevationError) and RKRedColor or (x & RKPositionFlagElevationEnabled) and RKGreenColor or RKYellowColor

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 48
def RKErrnoString(A):
    return (errno == EAGAIN) and 'EAGAIN' or (errno == EBADF) and 'EBADF' or (errno == EFAULT) and 'EFAULT' or (errno == EINTR) and 'EINTR' or (errno == EINVAL) and 'EINVAL' or (errno == ECONNREFUSED) and 'ECONNREFUSED' or (errno == EHOSTDOWN) and 'EHOSTDOWN' or (errno == EHOSTUNREACH) and 'EHOSTUNREACH' or (errno == EACCES) and 'EACCES' or (errno == EIO) and 'EIO' or 'OTHERS'

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMisc.h: 107
try:
    RKMiscStringLength = 1024
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 17
try:
    RKDefaultLogfile = 'messages.log'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 18
try:
    RKEOL = '\\r\\n'
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 21
def RKNextNModuloS(i, N, S):
    return (i >= (S - N)) and ((i + N) - S) or (i + N)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 22
def RKPreviousNModuloS(i, N, S):
    return (i < N) and ((S + i) - N) or (i - N)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 25
def RKNextModuloS(i, S):
    return (i == (S - 1)) and 0 or (i + 1)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 26
def RKPreviousModuloS(i, S):
    return (i == 0) and (S - 1) or (i - 1)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 29
def RKModuloLag(h, t, S):
    return (h < t) and ((h + S) - t) or (h - t)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 67
def RKRho2Uint8(r):
    return (roundf ((r > 0.93) and ((r * 1000.0) - 824.0) or (r > 0.7) and ((r * 300.0) - 173.0) or (r * 52.8571)))

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 69
def RKSingleWrapTo2PI(x):
    return (x < (-M_PI)) and (x + (2.0 * M_PI)) or (x >= M_PI) and (x - (2.0 * M_PI)) or x

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 71
def RKInstructIsAzimuth(i):
    return ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisAzimuth)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 72
def RKInstructIsElevation(i):
    return ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisElevation)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 73
def RKInstructIsNone(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeNone)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 74
def RKInstructIsTest(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeTest)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 75
def RKInstructIsSlew(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeSlew)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 76
def RKInstructIsPoint(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModePoint)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 77
def RKInstructIsStandby(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeStandby)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 78
def RKInstructIsDisable(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeDisable)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 79
def RKInstructIsEnable(i):
    return ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeEnable)

# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 17
try:
    RKCommonFFTPlanCount = 18
except:
    pass

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

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 15
try:
    RKRawDataRecorderDefaultMaximumRecorderDepth = 100000
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 16
try:
    RKRawDataRecorderDefaultCacheSize = ((32 * 1024) * 1024)
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 21
try:
    RKMomentDFTPlanCount = 16
except:
    pass

# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 12
try:
    RKSweepScratchSpaceDepth = 4
except:
    pass

# headers/RadarKit/RKTest.h: 15
try:
    RKTestWaveformCacheCount = 2
except:
    pass

rk_int16c = struct_rk_int16c# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 225

rk_complex = struct_rk_complex# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 233

rk_iqz = struct_rk_iqz# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 241

rk_modulo_path = struct_rk_modulo_path# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 251

rk_four_byte = union_rk_four_byte# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 265

rk_half_float_t = union_rk_half_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 278

rk_single_float_t = union_rk_single_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 292

rk_double_float_t = union_rk_double_float_t# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 306

rk_filter_anchor = union_rk_filter_anchor# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 324

rk_radarhub_ray_header = union_rk_radarhub_ray_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1144

rk_radarhub_ray = union_rk_radarhub_ray# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1152

rk_radar_desc = struct_rk_radar_desc# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1197

rk_waveform = struct_rk_waveform# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1210

rk_wave_file_header = union_rk_wave_file_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1223

rk_waveform_cal = struct_rk_waveform_cal# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1232

rk_waveform_response = struct_rk_waveform_response# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1239

rk_config = union_rk_config# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1275

rk_heath = union_rk_heath# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1289

rk_nodal_health = struct_rk_nodal_health# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1298

rk_position = union_rk_position# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1332

rk_scan_action = struct_rk_scan_action# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1339

rk_pulse_header = union_rk_pulse_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1367

rk_pulse_parameters = struct_rk_pulse_parameters# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1377

rk_pulse = struct_rk_pulse# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1393

rk_ray_header = struct_rk_ray_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1425

rk_ray = struct_rk_ray# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1438

rk_sweep_header = struct_rk_sweep_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1458

rk_sweep = struct_rk_sweep# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1467

rk_file_header = union_rk_file_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1479

rk_preferene_object = struct_rk_preferene_object# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1493

rk_control = struct_rk_control# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1504

rk_status = struct_rk_status# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1528

rk_simple_engine = struct_rk_simple_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1542

rk_file_monitor = struct_rk_file_monitor# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1556

rk_product_desc = union_rk_product_desc# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1580

rk_product_header = union_rk_product_header# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1615

rk_product = struct_rk_product# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1630

rk_product_collection = struct_rk_product_collection# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1635

rk_iir_filter = struct_rk_iir_filter# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1644

rk_task = struct_rk_task# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1650

rk_command_queue = struct_rk_command_queue# /mnt/k/OU/RadarKit/headers/RadarKit/RKTypes.h: 1662

RKGlobalParameterStruct = struct_RKGlobalParameterStruct# /mnt/k/OU/RadarKit/headers/RadarKit/RKFoundation.h: 92

rk_fft_resource = struct_rk_fft_resource# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 26

rk_fft_module = struct_rk_fft_module# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 35

rk_gaussian = struct_rk_gaussian# /mnt/k/OU/RadarKit/headers/RadarKit/RKDSP.h: 41

rk_compression_scratch = struct_rk_compression_scratch# headers/RadarKit/RKScratch.h: 52

rk_moment_scratch = struct_rk_moment_scratch# headers/RadarKit/RKScratch.h: 110

rk_pulse_worker = struct_rk_pulse_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 22

rk_pulse_engine = struct_rk_pulse_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseEngine.h: 40

rk_data_recorder = struct_rk_data_recorder# /mnt/k/OU/RadarKit/headers/RadarKit/RKRawDataRecorder.h: 20

rk_moment_worker = struct_rk_moment_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 26

rk_moment_engine = struct_rk_moment_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKMomentEngine.h: 41

rk_sweep_scratch = struct_rk_sweep_scratch# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 26

rk_sweep_engine = struct_rk_sweep_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKSweepEngine.h: 30

rk_pulse_ring_filter_worker = struct_rk_pulse_ring_filter_worker# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 21

rk_pulse_ring_filter_engine = struct_rk_pulse_ring_filter_engine# /mnt/k/OU/RadarKit/headers/RadarKit/RKPulseRingFilter.h: 39

rk_test_transceiver = struct_rk_test_transceiver# headers/RadarKit/RKTest.h: 70

rk_test_pedestal = struct_rk_test_pedestal# headers/RadarKit/RKTest.h: 94

rk_test_health_relay = struct_rk_test_health_relay# headers/RadarKit/RKTest.h: 106

# No inserted files

# No prefix-stripping

