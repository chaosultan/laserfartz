#!/bin/bash

# Bluetooth speaker MAC address
SPEAKER_MAC="F4:4E:FD:21:7C:54"

# Log file for debugging
LOG_FILE="/var/log/bt-speaker-connect.log"

# Function to log messages
log_message() {
    echo "$(date): $1" >> $LOG_FILE
}

log_message "Starting Bluetooth speaker connection script"

# Wait for Bluetooth service to be fully ready
sleep 5

# Check if Bluetooth is powered on
if ! bluetoothctl show | grep -q "Powered: yes"; then
    log_message "Bluetooth not powered on, powering on..."
    bluetoothctl power on
    sleep 2
fi

# Try to connect with retries
MAX_RETRIES=5
RETRY_COUNT=0
CONNECTED=false

while [ $RETRY_COUNT -lt $MAX_RETRIES ] && [ "$CONNECTED" = false ]; do
    RETRY_COUNT=$((RETRY_COUNT + 1))
    log_message "Connection attempt $RETRY_COUNT of $MAX_RETRIES"
    
    # Check if already connected
    if bluetoothctl info $SPEAKER_MAC | grep -q "Connected: yes"; then
        log_message "Already connected to speaker"
        CONNECTED=true
        break
    fi
    
    # Try to connect
    bluetoothctl connect $SPEAKER_MAC
    sleep 3
    
    # Verify connection
    if bluetoothctl info $SPEAKER_MAC | grep -q "Connected: yes"; then
        log_message "Successfully connected to speaker"
        CONNECTED=true
        break
    else
        log_message "Connection attempt $RETRY_COUNT failed"
        
        # If not the last attempt, try to reset and retry
        if [ $RETRY_COUNT -lt $MAX_RETRIES ]; then
            log_message "Resetting Bluetooth..."
            bluetoothctl disconnect $SPEAKER_MAC
            bluetoothctl power off
            sleep 2
            bluetoothctl power on
            sleep 3
        fi
    fi
done

if [ "$CONNECTED" = false ]; then
    log_message "Failed to connect after $MAX_RETRIES attempts"
else
    log_message "Connection process completed successfully"
fi
