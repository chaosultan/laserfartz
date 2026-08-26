#!/usr/bin/env python3
import serial
import json
import time
import subprocess
import os
from datetime import datetime

# Configure serial port
SERIAL_PORT = '/dev/ttyS0'  # Change this to match your setup
BAUD_RATE = 115200

# Alert configuration
ALERT_THRESHOLD = 100  # Value below this triggers alert
LASER_IDLE_VALUE = 4095  # Value when laser is not hitting sensor
ACTIVE_VALUE_THRESHOLD = 99  # Value below this means laser is active
ALERT_COOLDOWN = 2  # Minimum seconds between alerts

# Sound files
SYSTEM_UP_SOUND = 'fart_04.mp3'  # Played when laser first becomes active
ALERT_SOUND = 'fart_03.mp3'  # Played for subsequent state changes
LOG_FILE = 'alerts.log'

class ESPNowReceiverWithAlerts:
    def __init__(self, port=SERIAL_PORT, baud=BAUD_RATE):
        try:
            self.ser = serial.Serial(port, baud, timeout=1)
            print(f"Connected to {port} at {baud} baud")
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")
            print("Try: sudo chmod 666 /dev/ttyUSB0")
            exit(1)
        
        self.buffer = ""
        self.system_initialized = False  # Track if system has been initialized
        self.data = {
            'rec01': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True  # Track if this is the first activation
            },
            'rec02': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
            'rec03': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
            'rec04': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
            'rec05': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
            'rec06': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
            'rec07': {
                'value': 4095, 
                'connected': False, 
                'last_alert': 0,
                'previous_value': 4095,
                'laser_active': False,
                'alert_triggered': False,
                'first_activation': True
            },
        }
        
        self.json_error_count = 0
        self.max_json_errors = 10
        
        self.check_mpg123()
        self.check_sound_files()
    
    def check_mpg123(self):
        try:
            subprocess.run(['which', 'mpg123'], check=True, capture_output=True)
            print("✅ mpg123 is installed")
        except subprocess.CalledProcessError:
            print("⚠️ mpg123 is not installed!")
            print("Install it with: sudo apt install mpg123")
    
    def check_sound_files(self):
        """Check if sound files exist"""
        # Check system up sound
        if os.path.exists(SYSTEM_UP_SOUND):
            print(f"✅ System up sound found: {SYSTEM_UP_SOUND}")
        else:
            print(f"⚠️ System up sound not found: {SYSTEM_UP_SOUND}")
            print(f"Please place {SYSTEM_UP_SOUND} in the current directory")
        
        # Check alert sound
        if os.path.exists(ALERT_SOUND):
            print(f"✅ Alert sound found: {ALERT_SOUND}")
        else:
            print(f"⚠️ Alert sound not found: {ALERT_SOUND}")
            print(f"Please place {ALERT_SOUND} in the current directory")
    
    def log_event(self, device_name, event_type, value):
        """Log events to file"""
        try:
            with open(LOG_FILE, 'a') as f:
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                f.write(f"{timestamp},{device_name},{event_type},{value}\n")
        except Exception as e:
            print(f"Error writing to log: {e}")
    
    def play_sound(self, sound_file, device_name, event_type, value):
        """Play a sound file"""
        if not os.path.exists(sound_file):
            print(f"❌ Sound file not found: {sound_file}")
            return False
        
        try:
            subprocess.Popen(['mpg123', sound_file], 
                           stdout=subprocess.DEVNULL, 
                           stderr=subprocess.DEVNULL)
            print(f"🔊 Playing {sound_file} for {device_name} - {event_type}")
            return True
        except FileNotFoundError:
            print("❌ mpg123 not found")
            return False
        except Exception as e:
            print(f"❌ Error playing sound: {e}")
            return False
    
    def play_alert(self, device_name, value, event_type):
        """Play the appropriate alert sound based on event type and state"""
        current_time = time.time()
        last_alert = self.data[device_name]['last_alert']
        
        # Check cooldown to prevent spam
        if current_time - last_alert < ALERT_COOLDOWN:
            return
        
        print(f"🔊 {event_type}! {device_name} value changed to {value}")
        self.log_event(device_name, event_type, value)
        
        # Determine which sound to play
        sound_to_play = None
        
        if event_type == "FIRST_ACTIVATION":
            # First time laser becomes active - play system up sound
            sound_to_play = SYSTEM_UP_SOUND
            self.system_initialized = True
            print(f"🎯 SYSTEM INITIALIZED! First laser activation detected")
        elif event_type == "LASER_ACTIVE":
            # Laser became active again (after first time)
            sound_to_play = ALERT_SOUND
        elif event_type == "LASER_STOPPED":
            # Laser stopped
            sound_to_play = ALERT_SOUND
        else:
            sound_to_play = ALERT_SOUND
        
        # Play the sound
        if sound_to_play:
            success = self.play_sound(sound_to_play, device_name, event_type, value)
            if success:
                self.data[device_name]['last_alert'] = current_time
    
    def check_laser_state(self, device_name, current_value):
        """Check if laser state has changed and trigger alerts"""
        device_data = self.data[device_name]
        previous_value = device_data['previous_value']
        
        # Only process if device is connected
        if not device_data['connected']:
            return
        
        # Check for state changes
        # Laser becomes active: value goes from 4095 to below 99
        if (previous_value >= 4000 and current_value < ACTIVE_VALUE_THRESHOLD):
            print(f"🎯 LASER ACTIVE! {device_name} value: {previous_value} -> {current_value}")
            
            # Check if this is the first activation for this device
            if device_data['first_activation']:
                event_type = "FIRST_ACTIVATION"
                device_data['first_activation'] = False
                device_data['laser_active'] = True
                device_data['alert_triggered'] = True
                self.play_alert(device_name, current_value, event_type)
            else:
                event_type = "LASER_ACTIVE"
                device_data['laser_active'] = True
                device_data['alert_triggered'] = True
                self.play_alert(device_name, current_value, event_type)
        
        # Laser becomes inactive: value goes from below 99 to 4095
        elif (previous_value < ACTIVE_VALUE_THRESHOLD and current_value >= 4000):
            print(f"🚫 LASER STOPPED! {device_name} value: {previous_value} -> {current_value}")
            event_type = "LASER_STOPPED"
            device_data['laser_active'] = False
            device_data['alert_triggered'] = True
            self.play_alert(device_name, current_value, event_type)
        
        # Value changed but not a state change (just log it)
        elif previous_value != current_value:
            print(f"📊 {device_name} value changed: {previous_value} -> {current_value}")
        
        # Update previous value for next comparison
        device_data['previous_value'] = current_value
    
    def clean_json_string(self, json_str):
        """Attempt to clean and fix common JSON errors"""
        import re
        json_str = re.sub(r',\s*}', '}', json_str)
        json_str = re.sub(r',\s*]', ']', json_str)
        json_str = json_str.replace('\x00', '')
        return json_str
    
    def parse_json_safely(self, json_str):
        """Safely parse JSON with error recovery"""
        try:
            cleaned_json = self.clean_json_string(json_str)
            return json.loads(cleaned_json)
        except json.JSONDecodeError as e:
            print(f"⚠️ JSON Parse Error: {e}")
            self.json_error_count += 1
            
            try:
                import re
                match = re.search(r'\{[^{}]*\}', json_str)
                if match:
                    valid_json = match.group()
                    return json.loads(valid_json)
            except:
                pass
            
            return None
    
    def read_data(self):
        """Read and parse data from ESP32"""
        if self.ser.in_waiting > 0:
            data = self.ser.read(self.ser.in_waiting)
            
            try:
                decoded_data = data.decode('utf-8', errors='ignore')
            except:
                decoded_data = data.decode('latin-1', errors='ignore')
            
            self.buffer += decoded_data
            
            if len(self.buffer) > 10000:
                print("⚠️ Buffer too large, clearing...")
                self.buffer = ""
                return False
            
            processed = False
            
            while '===ESP_NOW_DATA_START===' in self.buffer:
                start_marker = '===ESP_NOW_DATA_START==='
                end_marker = '===ESP_NOW_DATA_END==='
                
                start_pos = self.buffer.find(start_marker)
                if start_pos == -1:
                    break
                
                end_pos = self.buffer.find(end_marker, start_pos + len(start_marker))
                if end_pos == -1:
                    break
                
                message_start = start_pos + len(start_marker)
                message = self.buffer[message_start:end_pos].strip()
                
                self.buffer = self.buffer[end_pos + len(end_marker):]
                
                if message:
                    if message.startswith('{'):
                        try:
                            parsed_data = self.parse_json_safely(message)
                            if parsed_data:
                                self.process_json_data(parsed_data)
                                self.json_error_count = 0
                            else:
                                print(f"⚠️ Failed to parse JSON data")
                                self.json_error_count += 1
                        except Exception as e:
                            print(f"❌ Error processing JSON: {e}")
                            self.json_error_count += 1
                    else:
                        try:
                            self.process_csv_data(message)
                            self.json_error_count = 0
                        except Exception as e:
                            print(f"❌ Error processing CSV: {e}")
                            self.json_error_count += 1
                    
                    if self.json_error_count > self.max_json_errors:
                        print(f"⚠️ Too many parse errors ({self.json_error_count}), resetting buffer...")
                        self.buffer = ""
                        self.json_error_count = 0
                    
                    processed = True
            
            return processed
        return False
    
    def process_json_data(self, data):
        """Process valid JSON data"""
        try:
            timestamp = data.get('timestamp', 0)
            devices = data.get('devices', {})
            
            print(f"\n[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}]")
            
            for device_name, device_data in devices.items():
                if device_name in self.data:
                    value = device_data.get('value', -1)
                    connected = device_data.get('connected', False)
                    
                    # Update data
                    self.data[device_name]['value'] = value
                    self.data[device_name]['connected'] = connected
                    
                    # Check laser state changes
                    if connected and value != -1:
                        self.check_laser_state(device_name, value)
                    
                    # Display status
                    status = "ONLINE" if connected else "OFFLINE"
                    laser_status = ""
                    if connected and value != -1:
                        if value < ACTIVE_VALUE_THRESHOLD:
                            laser_status = " 🎯 LASER ACTIVE"
                            if self.data[device_name]['first_activation']:
                                laser_status += " (FIRST TIME!)"
                        elif value >= 4000:
                            laser_status = " ⚪ LASER IDLE"
                        else:
                            laser_status = f" 📊 {value}"
                    
                    print(f"  {device_name}: {status} - Value: {value}{laser_status}")
                    
        except Exception as e:
            print(f"❌ Error processing JSON data: {e}")
    
    def process_csv_data(self, csv_str):
        """Process CSV data"""
        try:
            parts = csv_str.split(',')
            if len(parts) >= 9:
                values = parts[2:]
                
                print(f"\n[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}]")
                
                device_names = ['rec01', 'rec02', 'rec03', 'rec04', 'rec05', 'rec06', 'rec07']
                for i, name in enumerate(device_names):
                    if i < len(values) and name in self.data:
                        try:
                            value = int(values[i])
                            self.data[name]['value'] = value
                            self.data[name]['connected'] = (value != -1)
                            
                            # Check laser state changes
                            if value != -1:
                                self.check_laser_state(name, value)
                            
                            # Display status
                            status = "ONLINE" if value != -1 else "OFFLINE"
                            laser_status = ""
                            if value != -1:
                                if value < ACTIVE_VALUE_THRESHOLD:
                                    laser_status = " 🎯 LASER ACTIVE"
                                    if self.data[name]['first_activation']:
                                        laser_status += " (FIRST TIME!)"
                                elif value >= 4000:
                                    laser_status = " ⚪ LASER IDLE"
                                else:
                                    laser_status = f" 📊 {value}"
                            
                            print(f"  {name}: {status} - Value: {value}{laser_status}")
                        except ValueError:
                            pass
                        
        except Exception as e:
            print(f"❌ Error processing CSV: {e}")
    
    def close(self):
        self.ser.close()
        print("Serial connection closed")

# Main execution
if __name__ == "__main__":
    receiver = ESPNowReceiverWithAlerts()
    
    try:
        print("\n" + "="*60)
        print("LASER DETECTION SYSTEM with ESP-NOW")
        print(f"Laser active threshold: < {ACTIVE_VALUE_THRESHOLD}")
        print(f"Laser idle value: {LASER_IDLE_VALUE}")
        print(f"Alert cooldown: {ALERT_COOLDOWN} seconds")
        print("="*60 + "\n")
        
        print("🎵 First laser activation: " + SYSTEM_UP_SOUND + " (System ready)")
        print("🎵 Subsequent alerts: " + ALERT_SOUND)
        print("="*60 + "\n")
        
        print("Listening for ESP-NOW data...")
        print("🎯 Laser Active = value below 99")
        print("⚪ Laser Idle = value 4095")
        print("Press Ctrl+C to exit\n")
        
        while True:
            try:
                if receiver.read_data():
                    pass
            except serial.SerialException as e:
                print(f"❌ Serial error: {e}")
                print("Attempting to reconnect...")
                time.sleep(2)
                try:
                    receiver.ser.close()
                    receiver.ser.open()
                    print("✅ Reconnected to serial port")
                except:
                    print("❌ Failed to reconnect, exiting...")
                    break
            except Exception as e:
                print(f"❌ Unexpected error: {e}")
                time.sleep(1)
            
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        receiver.close()
