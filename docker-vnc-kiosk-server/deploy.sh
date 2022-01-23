#!/usr/bin/env bash
./stop.sh
./build.sh && docker-compose up -d
