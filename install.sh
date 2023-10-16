#!bin/sh

ARCH=$(uname -m)
VERSION=0.1.0
wget https://github.com/sesajad/successor/releases/download/v${VERSION}/successor-${VERSION}-${ARCH}.tar.gz -o /tmp/succ.tar.gz

tar -xzf /tmp/succ.tar.gz -C /tmp/succ
mkdir /succ
mkdir /succ/bin
mv /sbin/init /sbin/init2
cp /tmp/succ/ /succ/bin/
ln -s /succ/bin/init /sbin/init