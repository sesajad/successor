#!/bin/sh

mkdir -p /succ/new
docker build -t the-image . -o local,/succ/new
