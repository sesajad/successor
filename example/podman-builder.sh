#!/bin/sh

podman build -t the-image . -o type=local,dest=./rootfs/

rsync -n -aAXUNHxv --numeric-ids --delete --exclude={'dev','proc',sys','run','boot','tmp','home','root'} ./rootfs/ /
