Relation between nss, moz, moz_prebuilt
---------------------------------------
nss contains the security libraries which are also part of moz. However nss is
meant to be more current, that is, it to be updated more often. This 
should be easier than updating moz.

When nss is build, it depends on an environment variable (ENABLE_NSS_MODULE)
which is by default set to YES. In this case nss is build before moz. The nss
libraries/lib files/headers built in moz are then not delivered. Otherwise they
would overwrite those from nss. The nss libraries build in moz are then
removed from mozruntime.zip (build in moz/solver/bin), they are removed from the
lib directory (for example moz/unxlngi6.pro/lib), and the nss and nspr headers
are also removed (inc/nss and inc/nspr).  The nss libraries from the nss module
are then added to mozruntime.zip.

This also applies for moz_prebuilt. Therefore moz and moz_prebuilt must be build
again after changes have been made to the libraries in the nss module.

Also when moz was updated to use a newer version of mozilla, then one must make
sure that new files which also belong to nss are not delivered and are removed
from mozruntime.zip.

Fips 140 and signed libraries
-------------------------------
Fips 140 mode is not supported. That is, the *.chk files containing the
checksums for the cryptographic module are not delivered into solver and will
not be part of the OOo installation sets.

Signing has been turned off because 
- we change the rpath (install names) after signing which breaks the signatures
(Mac)
- sqlite conflicts with the system sqlite when signing which breaks the build


libfreebl3
----------
Porting to other platforms may require to deliver other variants of
libfreebl*. The library name varies according to the platform. Changes need to
be made to 
ooo/moz/extractfiles.mk
ooo/moz/zipped/makefile.mk
sun/moz_prebuilt/zipped/makefile.mk

See also
http://www.mozilla.org/projects/security/pki/nss/tech-notes/tn6.html


Windows builds of nss
---------------------
To build nss on Windows you need the MozillaBuild tools.

Wiki page containing the link to the build tools:
https://wiki.mozilla.org/MozillaBuild#Overview

The direct link (latest supported version):
https://ftp.mozilla.org/pub/mozilla/libraries/win32/MozillaBuildSetup-3.4.exe

libsqlite3
----------
The problem described here was found on Mac with OS 10.6
NSS cannot use the system sqlite on Mac because the base line is still MacOS
10.4. That system sqlite is incompatible with the softokn3 in nss which requires
a later version of sqlite. 
When we used a more current Mac SDK then we could set 
NSS_USE_SYSTEM_SQLITE=1
to build using the system sqlite.

We cannot deliver sqlite in the lib directory of the solver. This directory is
used by tools of the build environment. Using the sqlite from NSS breaks the
tools if they use system libraries which are linked with the system
sqlite. Therefore we deliver it into lib/sqlite on unix systems.

See also issue 
http://qa.openoffice.org/issues/show_bug.cgi?id=106132
