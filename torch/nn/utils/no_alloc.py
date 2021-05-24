from typing import Type

from ..._C import _suspend_allocators, _restore_allocators
from ..modules import Module


def no_alloc(module_cls: Type[Module], *args, **kwargs):
    _suspend_allocators()

    try:
        module = module_cls(*args, **kwargs)
    finally:
        _restore_allocators()

    return module
