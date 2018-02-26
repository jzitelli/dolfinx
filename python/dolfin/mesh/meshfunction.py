# Copyright (C) 2017 Chris N. Richardson and Garth N. Wells
#
# This file is part of DOLFIN (https://www.fenicsproject.org)
#
# SPDX-License-Identifier:    LGPL-3.0-or-later

import dolfin.cpp as cpp


_meshfunction_types = {"bool": cpp.mesh.MeshFunctionBool,
                       "size_t": cpp.mesh.MeshFunctionSizet,
                       "int": cpp.mesh.MeshFunctionInt,
                       "double": cpp.mesh.MeshFunctionDouble}


class MeshFunction(object):
    def __new__(cls, value_type, mesh, dim, value=None):
        if value_type not in _meshfunction_types.keys():
            raise KeyError("MeshFunction type not recognised")
        fn = _meshfunction_types[value_type]
        if value is not None:
            return fn(mesh, dim, value)
        else:
            return fn(mesh, dim)
