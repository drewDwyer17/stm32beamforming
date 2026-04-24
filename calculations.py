import math 
def calculate_minimum_toggle_period():
    #calculate the period that we want to toggle at to achieve a maximum frequency of togglign pins 
    maximum_clock_speed = 48000000 #48MHz
    maximum_clock_speed_in_ns = maximum_clock_speed * 1e-9 #48MHz = 48ns per clock cycle
    cpu_cycle_time_ns = 1 / maximum_clock_speed #20.833333333333332ns per clock cycle
    # One full square wave requires HIGH + LOW
    minimal_toggle_period_in_ns = 2 * cpu_cycle_time_ns #41.666666666666664ns for a full square wave period
    return minimal_toggle_period_in_ns


# during initial testing of MCU communication with the VGA, observed a lp looking loss pattern in the data signal 
# calculation of cutoff frequency to check if the loss was due to RC lp filter in circuitry
# it was.
# Removed the resistor+capacitor comprising the lp filter from the board to remove the issue 
def vga_cutoff_freq_lp_filter(): 
    signalFrequencyMOSI = 56179000000; # 5.617 mhz signal observed from oscilloscope 
    R7  = 4700 #ohms
    C13 =  2 #pF
    cutOffFrequency = 1/(2*math.pi*R7*C13) 
    return cutOffFrequency; 


# Calculate the phase shift needed between adjacent array elements
# to maximize gain for a signal arriving from a given angle.

# angle_deg: arrival angle in degrees
# rx_el_arr_spacing_d: spacing between adjacent antenna elements in meters
# signalFrequency: signal frequency in Hz
#depending on which element you are receiving at (index in the array of elements), you'll need to scale this required phase shift, because the distance traveled 
# will increase by L for each element you are receiving at. 

# returns: phase shift in radians
def calculateShiftForMaxGainAtGivenAngle(angle_deg, rx_el_arr_spacing_d, signalFrequency):
    angle_rad = math.radians(angle_deg)
    # extra distance to next element
    L = rx_el_arr_spacing_d * math.sin(angle_rad)
    speedOfLight = 3e8
    timeDelayToReachNextArrEl = L / speedOfLight
    phaseShift_to_maximize_gain = 2 * math.pi * signalFrequency * timeDelayToReachNextArrEl
    phaseShift_to_maximize_gain_deg = phaseShift_to_maximize_gain*/(2*pi)

    return phaseShift_to_maximize_gain_deg


if __name__ == "__main__":
    
    # _minimal_toggle_period_in_ns = calculate_minimum_toggle_period()
    # print(f"the minimal toggle period is {_minimal_toggle_period_in_ns}s")
    #the minimal toggle period is 2.0833333333333335e-08s
    #-------------------------------------------------------------------
    VGA_MOSI_cutoffFreq = vga_cutoff_freq_lp_filter()
    print(f"cutoffFreq = {VGA_MOSI_cutoffFreq}")
    # cutoffFreq = 16.931376924669717 Mhz     




