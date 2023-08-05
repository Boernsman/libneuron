# This script copies the project files to the embedded target. It takes the IP address as the first call argument.

echo "$1"

USER="nymea"
TARGET_PATH="~/"
IP=$1

if [[ $IP =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Copying files to $(IP)"
else
  echo "No, or invalid IP address"
  exit -1
fi

rsync -vhra ../libneuron $USER@$IP:$TARGET_PATH --include='**.gitignore' --exclude='/.git' --filter=':- .gitignore' --delete-after
