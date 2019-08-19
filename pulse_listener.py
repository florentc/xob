#!/usr/bin/env python3
"""
"""
import pulsectl
import sys

SINK_ID = 0

def create_handle_event_function(sink_id):
    """Create a handle_event() function with a given sink ID."""
    def handle_event(event):
        """If an event's index matches the sink ID, kill the loop."""
        if event.index == sink_id:
            raise pulsectl.PulseLoopStop

    return handle_event

def get_current_volume(pulse_control, sink_id):
    """Get the current volume to the nearest integer."""
    return round(
        pulse_control.sink_list()[sink_id].volume.value_flat * 100,
    )

# Start of the program.
with pulsectl.Pulse() as pulse_control:
    try:
        pulse_control.event_mask_set("sink")
        pulse_control.event_callback_set(create_handle_event_function(SINK_ID))
        current_volume = get_current_volume(pulse_control, SINK_ID)
        currently_muted = pulse_control.sink_list()[SINK_ID].mute == 1

        while True:
            pulse_control.event_listen()
            new_volume = get_current_volume(pulse_control, SINK_ID)
            newly_muted = pulse_control.sink_list()[SINK_ID].mute == 1
            if new_volume != current_volume or newly_muted != currently_muted:
                print(new_volume, end="")
                if newly_muted:
                    print("!")
                else:
                    print("")
                current_volume = new_volume
                currently_muted = newly_muted
            sys.stdout.flush()
    except (pulsectl.PulseLoopStop, KeyboardInterrupt):
        pass
