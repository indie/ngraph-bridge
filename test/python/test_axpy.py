# ==============================================================================
#  Copyright 2018 Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# ==============================================================================
"""nGraph TensorFlow installation test

"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import sys
import time
import getpass

import numpy as np
import tensorflow as tf
from tensorflow.python.client import device_lib
from tensorflow.python import pywrap_tensorflow as py_tf
from tensorflow.python.framework import errors_impl

import tfgraphviz as tfg

print("TensorFlow version: ", tf.GIT_VERSION, tf.VERSION)

# Define LD_LIBRARY_PATH indicating where nGraph library is located fornow.
# Eventually this won't be needed as the library will be available in either
# the Python site-packages or some other means
import ctypes
lib = ctypes.cdll.LoadLibrary('libngraph_device.so')

# Now try out some computation
graph_location = "/tmp/" + getpass.getuser() + "/tensorboard-logs/test"
print('Saving graph to: %s' % graph_location)

train_writer = tf.summary.FileWriter(graph_location)

a = 5.0
x = tf.placeholder(tf.float32, [None, 3])
y = tf.placeholder(tf.float32, shape=(2, 3))

with tf.device("/device:XLA_CPU:0"):
    axpy = a * x + y

    # Save the graphdef
    config = tf.ConfigProto(
        allow_soft_placement=True,
        log_device_placement=False,
        inter_op_parallelism_threads=1)

    with tf.Session(config=config) as sess:
        print("Python: Running with Session")
        (result) = sess.run(
            (axpy), feed_dict={
                x: np.ones((2, 3)),
                y: np.ones((2, 3)),
            })
        print("result:", result)

    train_writer.add_graph(tf.get_default_graph())
    tf.train.write_graph(
        tf.get_default_graph(), '.', 'test_axpy.pbtxt', as_text=True)
    g = tfg.board(tf.get_default_graph())
    g.render(filename="./test_axpy")
