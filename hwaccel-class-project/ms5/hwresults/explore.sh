#!/bin/bash

for mat in 8 16 32
do
  for cp in 8 9 11
  do
    tag=${mat}x${cp}
    if [ -d runs/$tag ]
    then
      echo skipping $tag
      continue
    fi

    echo starting $tag
    sed -i "s/MUL_SIZE = [0-9]*,/MUL_SIZE = $mat,/" ../hw_vec/top.v
    sed -i "s/-period [0-9]*/-period $cp/" top.sdc
    flow.tcl -design . -tag $tag

  done
done
