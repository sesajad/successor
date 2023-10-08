#!/bin/sh

mkdir -p /succ/new
podman build -t the-image . -o local,/succ/new
