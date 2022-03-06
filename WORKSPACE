# Muduo - A reactor-based C++ network library for Linux
# Copyright (c) 2010, Shuo Chen.  All rights reserved.
# http://code.google.com/p/muduo/
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#   * Neither the name of Shuo Chen nor the names of other contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

workspace(name = "com_github_chenshuo_muduo")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

BAZEL_SKYLIB_VERSION = "1.1.1"  # 2021-09-27T17:33:49Z

BAZEL_SKYLIB_SHA256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d"

http_archive(
    name = "bazel_skylib",
    sha256 = BAZEL_SKYLIB_SHA256,
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
    ],
)

http_archive(
    name = "boost",  # 2021-08-05T01:30:05Z
    build_file = "@com_github_nelhage_rules_boost//:BUILD.boost",
    patch_cmds = ["rm -f doc/pdf/BUILD"],
    patch_cmds_win = ["Remove-Item -Force doc/pdf/BUILD"],
    sha256 = "5347464af5b14ac54bb945dc68f1dd7c56f0dad7262816b956138fc53bcc0131",
    strip_prefix = "boost_1_77_0",
    urls = [
        "https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.gz",
    ],
)

http_archive(
    name = "com_github_nelhage_rules_boost",  # 2021-08-27T15:46:06Z
    patch_cmds = ["sed -i 's/net_zlib_zlib/com_github_madler_zlib/g' BUILD.boost"],
    patch_cmds_win = [
        """$content = (Get-Content 'BUILD.boost') -replace "net_zlib_zlib", "com_github_madler_zlib"
Set-Content BUILD.boost -Value $content -Encoding UTF8
""",
    ],
    sha256 = "2d0b2eef7137730dbbb180397fe9c3d601f8f25950c43222cb3ee85256a21869",
    strip_prefix = "rules_boost-fce83babe3f6287bccb45d2df013a309fa3194b8",
    urls = [
        "https://github.com/nelhage/rules_boost/archive/fce83babe3f6287bccb45d2df013a309fa3194b8.tar.gz",
    ],
)

http_archive(
    name = "com_github_madler_zlib",  # 2017-01-15T17:57:23Z
    build_file = "//bazel/third_party/zlib:zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
)
