#!/bin/bash
#Kill the query service job



kill $(pgrep -l QueryService | awk '{print $1'})

echo "done..."
