#!/bin/sh

mkdir -p /succ/new
docker build -t the-image . -o type=local,dest=/succ/new
