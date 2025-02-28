#!/bin/bash
clang --version | head -1 | awk '{print $4}' | cut -d. -f1
