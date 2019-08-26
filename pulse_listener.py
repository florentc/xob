#!/usr/bin/env python3
"""Listen to volume events from pulseaudio.

Written by Florent Ch.
Lightly edited by Tiger Sachse.
"""
import pulsectl
import sys


def create_handle_event_function(sink_id):
    """Create a handle_event() function with a given sink ID."""
    def handle_event(event):
        """If an event's index matches the sink ID, kill the loop."""
        if event.index == sink_id:
            raise pulsectl.PulseLoopStop

    return handle_event


def get_current_volume(pulse_controller, sink_id):
    """Get the current volume to the nearest integer."""
    return round(pulse_controller.sink_list()[sink_id].volume.value_flat * 100)


def is_muted(pulse_controller, sink_id):
    """Determine if the sink is muted."""
    return pulse_controller.sink_list()[sink_id].mute == 1


# Start of the program.
SINK_ID = 0
with pulsectl.Pulse() as pulse_controller:
    try:
        pulse_controller.event_mask_set("sink")
        pulse_controller.event_callback_set(
            create_handle_event_function(SINK_ID),
        )
        current_volume = get_current_volume(pulse_controller, SINK_ID)
        currently_muted = is_muted(pulse_controller, SINK_ID)

        while True:
            pulse_controller.event_listen()
            new_volume = get_current_volume(pulse_controller, SINK_ID)
            newly_muted = is_muted(pulse_controller, SINK_ID)
            if current_volume != new_volume or currently_muted != newly_muted:
                current_volume = new_volume
                currently_muted = newly_muted
                print(current_volume, end="")
                if currently_muted:
                    print("!")
                else:
                    print("")
                sys.stdout.flush()
    except (pulsectl.PulseLoopStop, KeyboardInterrupt):
        pass
