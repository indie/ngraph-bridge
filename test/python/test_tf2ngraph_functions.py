# ==============================================================================
#  Copyright 2019 Intel Corporation
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
"""nGraph TensorFlow bridge test for helper functions of tf2ngraph tool

"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import pytest
import os
import numpy as np
import shutil
import tensorflow as tf
import ngraph_bridge

from tools.build_utils import command_executor
from tools.tf2ngraph import update_config_to_include_custom_config

from common import NgraphTest


class Testtf2ngraphHelperFunctions(NgraphTest):

    def test_config_updater_api(self):
        config = update_config_to_include_custom_config(tf.ConfigProto(), 'CPU',
                                                        '0', {
                                                            'abc': '1',
                                                            'def': '2'
                                                        }, [{
                                                            'x': [1]
                                                        }], True)
        assert config.HasField('graph_options')
        assert config.graph_options.HasField('rewrite_options')
        custom_opts = config.graph_options.rewrite_options.custom_optimizers
        assert len(custom_opts) == 1
        assert custom_opts[0].name == 'ngraph-optimizer'
        assert set(custom_opts[0].parameter_map.keys()) == {
            'abc', 'ngraph_backend', 'def', 'device_id', 'aot_requested',
            'shape_hint_0'
        }
        retrieved_dict = {}
        for key, val in custom_opts[0].parameter_map.items():
            # For everything other than shape_hint_0, the values are simple strings
            # shape_hint_0 contains a complex data structure representing shape hint
            if (key != "shape_hint_0"):
                retrieved_dict[key] = val.ListFields()[0][1].decode()
        assert retrieved_dict == {
            'abc': '1',
            'def': '2',
            'ngraph_backend': 'CPU',
            'device_id': '0',
            'aot_requested': '1'
        }
