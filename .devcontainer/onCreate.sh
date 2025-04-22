#!/bin/bash
set -euo pipefail
sudo pip install -r requirements.txt
simbricks-cli --install-completion
