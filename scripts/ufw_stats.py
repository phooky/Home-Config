#!/usr/bin/python3

import re

entry_src=r'^(?P<datetime>\w+ \d+ \d+:\d+:\d+) (?P<hostname>\w+) kernel: \[(?P<uptime>[0-9. ]+)\] \[(?P<evt_type>[A-Z ]+)\] (?P<tags>.*)$'

entry_re = re.compile(entry_src)

events = []

# parse out log entries
for entry in open('/var/log/ufw.log'):
    m = entry_re.match(entry)
    if m:
        event = m.groupdict()
        for tagdesc in event['tags'].split(' '):
            if not tagdesc: continue
            idx = tagdesc.find('=')
            if idx >= 0:
                event[tagdesc[:idx]] = tagdesc[idx+1:]
            else:
                event[tagdesc] = True
        events.append(event)

i
