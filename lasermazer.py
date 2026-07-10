import RPi.GPIO as GPIO
import time
import subprocess
import threading

# Use Broadcom (BCM) pin numbering
GPIO.setmode(GPIO.BCM)

# Set the pin number connected to the Pico (using pin 21)
RECEIVE_PIN = 21

# Set the pin as an input with a pull-up resistor
GPIO.setup(RECEIVE_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# Global flag to track if we're in the cooldown period
cooldown_active = False
cooldown_lock = threading.Lock()

def play_sound():
    """Play the sound immediately"""
    print("Playing gypsy_fart_04.wav...")
    try:
        subprocess.run(["aplay", "gypsy_fart_04.wav"], check=True)
        print("Playback complete.")
    except subprocess.CalledProcessError as e:
        print(f"Error playing sound: {e}")
    except FileNotFoundError:
        print("Error: 'aplay' command not found or file 'gypsy_fart_04.wav' doesn't exist")

def cooldown_timer():
    """Handle the 3-second cooldown period"""
    global cooldown_active
    print("Cooldown started: 3 seconds before next trigger allowed")
    time.sleep(3)
    with cooldown_lock:
        cooldown_active = False
    print("Cooldown ended: Ready for next trigger")

def my_interrupt_callback(channel):
    global cooldown_active
    
    # Check if cooldown is active
    with cooldown_lock:
        if cooldown_active:
            print(f"Interrupt received but IGNORED - cooldown active")
            return
    
    # If we get here, we can process the interrupt
    print(f"Interrupt received on channel {channel}!")
    
    # Set cooldown flag immediately
    with cooldown_lock:
        cooldown_active = True
    
    # Play sound immediately in a separate thread
    threading.Thread(target=play_sound, daemon=True).start()
    
    # Start the cooldown timer in a separate thread
    threading.Thread(target=cooldown_timer, daemon=True).start()

# Add an event detection for a FALLING edge
GPIO.add_event_detect(RECEIVE_PIN, GPIO.FALLING, callback=my_interrupt_callback, bouncetime=200)

print(f"Waiting for interrupt signal on GPIO pin {RECEIVE_PIN} from Pico...")
print("Sound will play immediately on trigger, then wait 3 seconds before allowing next trigger.")
print("Press Ctrl+C to exit.")

try:
    # Keep the program running
    while True:
        time.sleep(1)  # Do other tasks here if needed
except KeyboardInterrupt:
    print("\nExiting...")
    GPIO.cleanup()
