echo ">>>>>>>>>>>>>>>>>>Building mxe. This may take a while."
sudo apt-get install bzip2 cmake flex gcc g++ gperf intltool libtool ruby scons
cd mxe
make pkgconf
make binutils 
make gcc-gmp
make gcc-isl
make gcc-cloog
make gcc-mpfr
make gcc-mpc
make mingw-w64 
make gcc
make gzip2
make libiconv 
make gettext
make pcre
make zlib
make dbus
make glib
make lzo
make icu4c
make apr 
make apr-util 
make curl
make pthreads
make libwebsockets
export PATH=$PWD/usr/bin:$PATH
cp ./usr/i686-w64-mingw32.static/include/winioctl.h ./usr/i686-w64-mingw32.static/include/WinIoCtl.h
cp ./usr/i686-w64-mingw32.static/include/windows.h ./usr/i686-w64-mingw32.static/include/Windows.h
cd ../libuv
sh autogen.sh
./configure --host i686-w64-mingw32.static --disable-shared
echo ">>>>>>>>>>>>>>>>>>building libuv"  
make
make check
sudo make install
sudo cp /usr/local/lib/libuv.a ../libs/
cd ..
mkdir db_win
unzip db-6.1.19.zip -d db_win
cd db_win/db-6.1.19
echo ">>>>>>>>>>>>>>>>>>building libdb"
mkdir build_mxe
cd build_mxe
CC=i686-w64-mingw32.static-gcc CXX=i686-w64-mingw32.static-g++ ../dist/configure --enable-mingw --disable-replication --enable-cxx --host i686-w64-mingw32.static
make
cp libdb.a ../../../libs
cp db.h ../../../
cd ../../..
echo ">>>>>>>>>>>>>>>>>>building SuperNET.exe"
i686-w64-mingw32.static-gcc -o SuperNET.exe SuperNET.c picoc.c table.c lex.c parse.c expression.c heap.c type.c variable.c clibrary.c platform.c include.c platform/platform_unix.c platform/library_unix.c cstdlib/stdio.c cstdlib/math.c cstdlib/string.c cstdlib/stdlib.c cstdlib/time.c cstdlib/errno.c cstdlib/ctype.c cstdlib/stdbool.c cstdlib/unistd.c libgfshare.c libjl777.c gzip/adler32.c gzip/crc32.c gzip/gzclose.c gzip/gzread.c gzip/infback.c gzip/inflate.c gzip/trees.c gzip/zutil.c gzip/compress.c gzip/deflate.c gzip/gzlib.c gzip/gzwrite.c gzip/inffast.c  gzip/inftrees.c gzip/uncompr.c libtom/yarrow.c libtom/aes.c libtom/cast5.c libtom/khazad.c libtom/rc2.c libtom/safer.c libtom/skipjack.c libtom/aes_tab.c libtom/crypt_argchk.c libtom/kseed.c libtom/rc5.c libtom/saferp.c libtom/twofish.c libtom/anubis.c libtom/des.c libtom/multi2.c libtom/rc6.c libtom/safer_tab.c libtom/twofish_tab.c libtom/blowfish.c libtom/kasumi.c libtom/noekeon.c libtom/rmd160.c libtom/sha256.c libtom/xtea.c libs/libdb.a libs/libuv.a -lwebsockets -lssl -lcrypto -lpthread -lssh2 -lm libs/randombytes.c -lws2_32 -lwsock32 -lgdi32 -DCURL_STATICLIB  -lgcrypt -lwldap32 -liconv -lintl -lnettle -lpsapi -liphlpapi -lcurl -lws2_32 -lwldap32
strip SuperNET.exe
echo '>>>>>>>>>>>>>>>>>>finished'
