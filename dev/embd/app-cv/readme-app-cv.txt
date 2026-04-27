NPPM APP using shared object

(Needed: OpenCV lib)
===============================================
Project : /home/cj/Dropbox/2021-try-Prjct/er/robert/2-robert/
===============================================

$ pwd
/home/cj/Dropbox/2021-try-Prjct/er/robert/2-robert/dev/embd/app-cv


$ export PKG_CONFIG_PATH=/home/cj/Dropbox/2021-try-Prjct/cv/cv-install-here-cj-09May22/lib/pkgconfig/:$PKG_CONFIG_PATH

$ pkg-config --modversion opencv4
4.5.5

$ export LD_LIBRARY_PATH=/home/cj/Dropbox/2021-try-Prjct/er/robert/2-robert/dev/embd/sdk-nppm-4vbx-ubt2004/:$LD_LIBRARY_PATH


$ g++ \
../../../apps/nppm/main.cpp \
../../../apps/nppm/cast.cpp \
../../../apps/nppm/geo.cpp \
../../../apps/nppm/tools.cpp \
../../../apps/nppm/nppm.cpp \
`pkg-config --cflags --libs opencv4` \
-L /home/cj/Dropbox/2021-try-Prjct/er/robert/2-robert/dev/embd/sdk-nppm-4vbx-ubt2004/ \
-lubt2004nppm \
-o ubt-noncmake-nppm-demo

$ ./ubt-noncmake-nppm-demo 


4 Oracle Virtual Box
$ export LD_LIBRARY_PATH=/home/cj/cj2hisi/cj-appm-app/dev/embd/sdk-nppm-4vbx-ubt2004/:$LD_LIBRARY_PATH

$ cat /etc/ld.so.conf.d/opencv4.conf
/home/cj/Dropbox/2021-try-Prjct/cv/cv-install-here-cj-09May22/lib
$ sudo ldconfig



NPPM APP using shared object

(Needed: OpenCV lib)
===============================================
/home/cj/CLionProjects/nppm-app/
===============================================

$ pwd
/home/cj/CLionProjects/nppm-app/dev/embd/app-cv


$ export PKG_CONFIG_PATH=/home/cj/Dropbox/2021-try-Prjct/cv/cv-install-here-cj-09May22/lib/pkgconfig/:$PKG_CONFIG_PATH

$ pkg-config --modversion opencv4
4.5.5

$ export LD_LIBRARY_PATH=/home/cj/CLionProjects/nppm-app/dev/embd/sdk-nppm-4vbx-ubt2004/:$LD_LIBRARY_PATH


$ g++ \
../../../apps/nppm/main.cpp \
../../../apps/nppm/cast.cpp \
../../../apps/nppm/geo.cpp \
../../../apps/nppm/tools.cpp \
../../../apps/nppm/nppm.cpp \
`pkg-config --cflags --libs opencv4` \
-L /home/cj/CLionProjects/nppm-app/dev/embd/sdk-nppm-4vbx-ubt2004/ \
-lubt2004nppm \
-o ubt-noncmake-nppm-demo

$ ./ubt-noncmake-nppm-demo 

