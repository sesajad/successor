#!bin/sh

ARCH=$(uname -m)
VERSION=0.1.0

wget https://github.com/sesajad/successor/releases/download/v${VERSION}/successor-${ARCH}.tar.gz -O /tmp/succ.tar.gz && \
mkdir /tmp/succ && \
tar -xzf /tmp/succ.tar.gz -C /tmp/succ && \
rm -rf /tmp/succ.tar.gz && \
mkdir /succ && \
mkdir /succ/bin && \
mv /sbin/init /sbin/init2 && \
cp /tmp/succ/init /succ/bin/init && \
cp /tmp/succ/setroot /succ/bin/setroot && \
cp /tmp/succ/succ-build /succ/bin/succ-build && \
cp /tmp/succ/succ-clean /succ/bin/succ-clean && \
cp /tmp/succ/succ-current /succ/bin/succ-current && \
cp /tmp/succ/succ-uninstall /succ/bin/succ-uninstall && \
chmod +x /succ/bin/* && \
ln -s /succ/bin/init /sbin/init && \
rm -rf /tmp/succ