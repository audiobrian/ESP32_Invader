#!/bin/bash

# WiFi Configuration Setup Script for ESP32 Invader

CONFIG_FILE="include/wifi_config.h"
TEMPLATE_FILE="include/wifi_config.h.template"

echo "ESP32 WiFi Configuration Setup"
echo "============================="

if [ ! -f "$CONFIG_FILE" ]; then
    echo "Creating wifi_config.h from template..."
    cp "$TEMPLATE_FILE" "$CONFIG_FILE"
fi

echo "Current configuration:"
echo "SSID: $(grep 'WIFI_SSID' "$CONFIG_FILE" | cut -d'"' -f2)"
echo "Password: $(grep 'WIFI_PASSWORD' "$CONFIG_FILE" | cut -d'"' -f2)"
echo ""

read -p "Enter WiFi SSID: " ssid
read -s -p "Enter WiFi Password: " password
echo ""

# Update the config file
sed -i "s/define WIFI_SSID.*/define WIFI_SSID \"$ssid\"/" "$CONFIG_FILE"
sed -i "s/define WIFI_PASSWORD.*/define WIFI_PASSWORD \"$password\"/" "$CONFIG_FILE"

echo "WiFi configuration updated successfully!"
echo ""
echo "To secure for GitHub commit:"
echo "git update-index --skip-worktree $CONFIG_FILE"