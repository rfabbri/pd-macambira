#!/bin/sh
pd -alsa -alsadev hw:1,1 -inchannels 8 -outchannels 8 -frags 20 testbird.pd
