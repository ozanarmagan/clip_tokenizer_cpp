load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_foreign_cc",
    patches = ["//bazel:foreign_cc.patch"],
    sha256 = "2a4d07cd64b0719b39a7c12218a3e507672b82a97b98c6a89d38565894cf7c51",
    strip_prefix = "rules_foreign_cc-0.9.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/refs/tags/0.9.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

new_git_repository(
    name = "icu",
    build_file = "//bazel/icu:BUILD",
    remote = "https://github.com/unicode-org/icu.git",
    patches = ["//bazel/icu:icu.patch"],
    tag = "release-71-1",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_git_repository(
    name = "pybind11_bazel",
    remote = "https://github.com/pybind/pybind11_bazel.git",
    tag = "v2.11.1"
)

new_git_repository(
  name = "pybind11",
  build_file = "@pybind11_bazel//:pybind11.BUILD",
  remote = "https://github.com/pybind/pybind11.git",
  tag = "v2.11.1"
)

load("@pybind11_bazel//:python_configure.bzl", "python_configure")
python_configure(name = "local_config_python")
