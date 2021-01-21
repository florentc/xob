#!/usr/bin/env python3
from pulsectl import Pulse, PulseLoopStop
import pulsectl
import sys
import re


def get_default_sink(): 
    pulse = pulsectl.Pulse('local')
    sinks = pulse.sink_list()
    default_sink = pulse.server_info().default_sink_name

    sinktxt = str(sinks)
    splitsink = re.findall('\<(.*?)\>', sinktxt) 

    for i in splitsink:
        if default_sink in i:
            splitindex = i.split(', ')
            found_sink_id = (splitindex[1][6:])
            return found_sink_id

sink_id = get_default_sink()

with Pulse() as pulse:
  def callback(ev):
    if ev.index == sink_id:
        raise PulseLoopStop
  try:
    pulse.event_mask_set('sink')
    pulse.event_callback_set(callback)
    last_value = round(pulse.sink_list()[sink_id].volume.value_flat * 100)
    last_mute = pulse.sink_list()[sink_id].mute == 1
    while True:
      pulse.event_listen()
      value = round(pulse.sink_list()[sink_id].volume.value_flat * 100)
      mute = pulse.sink_list()[sink_id].mute == 1
      if value != last_value or mute != last_mute:
        print(value, end='')
        if mute:
            print('!')
        else:
            print('')
        last_value = value
        last_mute = mute
      sys.stdout.flush()
  except:
    pass
