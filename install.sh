#!bin/sh

ARCH=$(uname -m)
VERSION=0.2.0

wget https://github.com/sesajad/successor/releases/download/v${VERSION}/successor-${ARCH}.tar.gz -O /tmp/succ.tar.gz && \
mkdir -p /tmp/succ && \
tar -xzf /tmp/succ.tar.gz -C /tmp/succ && \
rm -rf /tmp/succ.tar.gz && \
mkdir -p /succ/bin && \
echo "Copying files..." && \
cp /tmp/succ/init /succ/bin/init && \
cp /tmp/succ/successor /succ/bin/successor && \
cp /tmp/succ/uninstall /succ/bin/uninstall && \
chmod +x /succ/bin/* && \
echo "Removing temporary files..." && \
rm -rf /tmp/succ

if [ $? -ne 0 ] ; then
  echo "Installation failed"
  exit 1
fi

ln -s ../succ/bin/successor /sbin/successor 
if [ -f /sbin/init2 ] ; then
  echo "Init script already installed"
  exit 1
else
  echo "Installing init script..."
  mv /sbin/init /sbin/init2 && \
  ln -s ../succ/bin/init /sbin/init
fi

echo "Done!"