FROM --platform=linux/amd64 centos:7
# CentOS 7 is EOL -> repoint yum at the vault archive
RUN sed -i -e 's|^mirrorlist=|#mirrorlist=|g' \
           -e 's|^#\s*baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' \
           /etc/yum.repos.d/CentOS-*.repo \
 && yum -y install gcc gcc-c++ make file which binutils \
                   glibc-devel.i686 libstdc++-devel.i686 zlib-devel.i686 \
 && yum clean all
# lzo compression headers (code does #include "lzo/lzoconf.h"); separate layer
# keeps the toolchain layer cached.
RUN yum -y install lzo-devel lzo-devel.i686 && yum clean all
WORKDIR /work
