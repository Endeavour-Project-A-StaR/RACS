import serial
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Button
from scipy.spatial.transform import Rotation as R

# --- CONFIGURATION ---
SERIAL_PORT = '/dev/ttyACM0'  # Change this to your Teensy's serial port!
BAUD_RATE = 115200

# Connect to the flight computer
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    print(f"Connected to {SERIAL_PORT}")
except Exception as e:
    print(f"Error opening serial port: {e}")
    print("Make sure the port is correct and the Arduino Serial Monitor is CLOSED.")
    exit()

# --- SETUP PLOT ---
fig = plt.figure(figsize=(12, 7))
fig.canvas.manager.set_window_title('Bayes 3.5 Ground Control Station')

# Adjust layout to make room for buttons at the bottom
plt.subplots_adjust(bottom=0.2)

# Left Plot: 3D Rocket Orientation
ax_3d = fig.add_subplot(121, projection='3d')
ax_3d.set_xlim([-1, 1])
ax_3d.set_ylim([-1, 1])
ax_3d.set_zlim([-1, 1])
ax_3d.set_xlabel('X')
ax_3d.set_ylabel('Y')
ax_3d.set_zlabel('Z')
ax_3d.set_title("Rocket Orientation (Nose Vector)")

# Draw a red line to represent the rocket's longitudinal axis (Nose)
line_rocket, = ax_3d.plot([0, 1], [0, 0], [0, 0], 'r-', linewidth=6, label='Rocket Nose (X-axis)')
ax_3d.legend()

# Right Plot: Servo Deflection Bar Chart
ax_bar = fig.add_subplot(122)
ax_bar.set_ylim([50, 130]) # Bounds slightly beyond your 60-120 limits for visibility
ax_bar.set_title("Live Servo Deflections")
ax_bar.set_ylabel("Degrees")

servo_labels = ['S1', 'S2', 'S3', 'S4']
bars = ax_bar.bar(servo_labels, [90, 90, 90, 90], color=['#FF595E', '#8AC926', '#1982C4', '#FFCA3A'])
ax_bar.axhline(90, color='black', linestyle='--', linewidth=1, label='Center (90°)')
ax_bar.legend()

# --- BUTTON CONTROLS ---
# Define axis positions for the buttons: [left, bottom, width, height]
ax_btn_p = plt.axes([0.25, 0.05, 0.2, 0.075])
ax_btn_o = plt.axes([0.55, 0.05, 0.2, 0.075])

btn_p = Button(ax_btn_p, 'PREFLIGHT (P)', color='#d3d3d3', hovercolor='#a9a9a9')
btn_o = Button(ax_btn_o, 'OVERRIDE (O)', color='#ff9999', hovercolor='#ff6666')

# Button Callback Functions
def send_preflight(event):
    if ser and ser.is_open:
        ser.write(b'P')
        print("Command Sent: PREFLIGHT (P) - Alignment active, servos locked.")

def send_override(event):
    if ser and ser.is_open:
        ser.write(b'O')
        print("Command Sent: OVERRIDE (O) - Gyro integration active, servos LIVE.")

# Attach the callbacks to the buttons
btn_p.on_clicked(send_preflight)
btn_o.on_clicked(send_override)

# --- ANIMATION LOOP ---
def update(frame):
    if ser.in_waiting:
        try:
            # Read the latest line from the serial buffer
            line = ser.readline().decode('utf-8').strip()
            
            # Skip boot messages or garbled data, only parse JSON
            if not line.startswith('{'):
                # Print non-JSON messages to console so you can see FC debug info
                if line: print(f"FC MSG: {line}")
                return line_rocket, *bars
            
            data = json.loads(line)
            
            # --- 1. Update 3D Orientation ---
            # Your FC sends Quats as: [w, x, y, z]
            # SciPy expects Quats as: [x, y, z, w]
            qw, qx, qy, qz = data['quats']
            rot = R.from_quat([qx, qy, qz, qw])
            
            # Apply the rotation to the base X-axis vector (1, 0, 0)
            nose_vector = rot.apply([1, 0, 0])
            
            # Update the 3D line coordinates
            line_rocket.set_data_3d([0, nose_vector[0]], 
                                    [0, nose_vector[1]], 
                                    [0, nose_vector[2]])
            
            # --- 2. Update Servo Bars ---
            servos = data['servo']
            for bar, val in zip(bars, servos):
                bar.set_height(val)
                
                # Turn the bar Red if it hits the maximum limit to warn you
                if val <= 60.1 or val >= 119.9:
                    bar.set_color('red')
                else:
                    bar.set_color('#1982C4') # Default blue
                
        except json.JSONDecodeError:
            pass # Ignore partially received lines
        except KeyError:
            pass # Ignore JSON missing our specific keys
            
    return line_rocket, *bars

# Run the animation at ~20 FPS (50ms interval) to match your FC telemetry rate
ani = animation.FuncAnimation(fig, update, interval=50, blit=False, cache_frame_data=False)

plt.show()