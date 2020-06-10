#!/usr/bin/python3
import enum
import uuid
from typing import Tuple

import torch
import torch.testing._internal.dist_utils as dist_utils
from torch import Tensor, nn
from torch._jit_internal import Future
from torch.distributed.nn.api.remote_module import (
    RemoteModule,
    _gen_global_unique_name,
    _RemoteModule,
)
from torch.testing._internal.distributed.rpc.rpc_agent_test_fixture import (
    RpcAgentTestFixture,
)


class ModuleCreationMode(enum.Enum):
    MODULE_CTOR_WITH_INTERFACE = "module_ctor_with_interface"
    MODULE_CTOR = "module_ctor"


@torch.jit.interface
class MyModuleInterface:
    def forward(
        self, tensor: Tensor, number: int, word: str = "default"
    ) -> Tuple[str, int, Tensor]:
        pass


@torch.jit.interface
class RemoteMyModuleInterface:
    def forward(
        self, tensor: Tensor, number: int, word: str = "default"
    ) -> Tuple[str, int, Tensor]:
        pass

    def forward_async(
        self, tensor: Tensor, number: int, word: str = "default"
    ) -> Future[Tuple[str, int, Tensor]]:
        pass


class MyModule(nn.Module):
    def __init__(self, first_arg, first_kwarg=-1):
        super().__init__()

    def forward(
        self, tensor: Tensor, number: int, word: str = "default"
    ) -> Tuple[str, int, Tensor]:
        return word, number, tensor


class NonModule:
    def __init__(self, first_arg, first_kwarg=-1):
        pass


def create_scripted_module(first_arg, first_kwarg=-1):
    module = MyModule(first_arg, first_kwarg=first_kwarg)
    scripted_module = torch.jit.script(module)
    return scripted_module


def bad_module_creator(first_arg, first_kwarg=-1):
    return NonModule(first_arg, first_kwarg=first_kwarg)


class RemoteModuleTest(RpcAgentTestFixture):
    @property
    def world_size(self):  # Override setting in RpcAgentTestFixture
        return 2

    def setUp(self):
        super().setUp()
        self._fork_processes()

    @dist_utils.dist_init
    def test_gen_global_unique_name(self):
        if self.rank != 0:
            return
        name_0 = _gen_global_unique_name()
        name_1 = _gen_global_unique_name()
        self.assertNotEqual(name_0, name_1)

    @staticmethod
    def _create_remote_module_iter(
        dst_worker_name, modes=None, name=None,
    ):
        if modes is None:
            modes = ModuleCreationMode.__members__.values()

        args = (1,)
        kwargs = dict(first_kwarg=2)

        if ModuleCreationMode.MODULE_CTOR in modes:
            remote_module = RemoteModule(
                dst_worker_name,
                MyModule,
                args,
                kwargs,
                name=name,
            )
            yield remote_module

        if ModuleCreationMode.MODULE_CTOR_WITH_INTERFACE in modes:
            remote_module = _RemoteModule(
                dst_worker_name,
                create_scripted_module,
                args,
                kwargs,
                name=name,
                _module_interface_cls=MyModuleInterface,
            )
            scripted_remote_module = torch.jit.script(remote_module)
            yield scripted_remote_module

    @dist_utils.dist_init
    def test_bad_module_creator(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)
        args = (1,)
        kwargs = dict(first_kwarg=2)

        with self.assertRaisesRegex(
            ValueError,
            r"Expect module_creator returns an instancee of <class nn.Module>,"
        ):
            RemoteModule(
                dst_worker_name,
                bad_module_creator,
                args,
                kwargs
            )

        with self.assertRaisesRegex(
            ValueError,
            r"Expect module_creator returns an instancee of <class nn.Module>,"
        ):
            RemoteModule(
                dst_worker_name,
                NonModule,
                args,
                kwargs
            )

    @dist_utils.dist_init
    def test_forward_async(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)
        args = (torch.ones(1), 2, "3")
        for remote_module in self._create_remote_module_iter(dst_worker_name):
            ret_fut = remote_module.forward_async(*args)
            ret = ret_fut.wait()
            self.assertEqual(ret, tuple(reversed(args)))

    @dist_utils.dist_init
    def test_forward_async_script(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)

        scripted_remote_module = next(
            self._create_remote_module_iter(
                dst_worker_name, modes=[ModuleCreationMode.MODULE_CTOR_WITH_INTERFACE]
            )
        )

        @torch.jit.script
        def run_forward_async(scripted_remote_module: RemoteMyModuleInterface):
            ret_fut = scripted_remote_module.forward_async(torch.ones(1), 2, "3")
            ret = ret_fut.wait()
            return ret

        ret = run_forward_async(scripted_remote_module)

        self.assertEqual(ret, ("3", 2, torch.ones(1)))

    @dist_utils.dist_init
    def test_forward_sync(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)
        args = (torch.ones(1), 2, "3")
        for remote_module in self._create_remote_module_iter(dst_worker_name):
            ret = remote_module.forward(*args)
            self.assertEqual(ret, tuple(reversed(args)))

    @dist_utils.dist_init
    def test_forward_sync_script(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)

        scripted_remote_module = next(
            self._create_remote_module_iter(
                dst_worker_name, modes=[ModuleCreationMode.MODULE_CTOR_WITH_INTERFACE]
            )
        )

        @torch.jit.script
        def run_forward(scripted_remote_module: MyModuleInterface):
            ret = scripted_remote_module.forward(torch.ones(1), 2, "3")
            return ret

        ret = run_forward(scripted_remote_module)

        self.assertEqual(ret, ("3", 2, torch.ones(1)))

    @dist_utils.dist_init
    def test_forward_with_kwargs(self):
        if self.rank != 0:
            return
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)
        args = (torch.ones(1), 2)
        kwargs = dict(word="3")
        # Only test Python nn.Module, because script module methods don't support taking kwargs.
        for remote_module in self._create_remote_module_iter(
            dst_worker_name, modes=[ModuleCreationMode.MODULE_CTOR]
        ):
            ret_fut = remote_module.forward_async(*args, **kwargs)
            ret = ret_fut.wait()
            self.assertEqual(ret, tuple(reversed(args + ("3",))))

            ret = remote_module.forward(*args, **kwargs)
            self.assertEqual(ret, tuple(reversed(args + ("3",))))

    @dist_utils.dist_init
    def test_user_provided_name(self):
        if self.rank != 0:
            return
        name = f"_remote_module_{uuid.uuid4().hex}"
        dst_worker_name = dist_utils.worker_name((self.rank + 1) % self.world_size)
        args = (torch.ones(1), 2, "3")
        for remote_module in self._create_remote_module_iter(
            dst_worker_name, name=name
        ):
            ret_fut = remote_module.forward_async(*args)
            ret = ret_fut.wait()
            self.assertEqual(ret, tuple(reversed(args)))
