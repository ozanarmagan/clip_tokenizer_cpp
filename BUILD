cc_library(
    name="clip",
    srcs=["clip_tokenizer.cpp"],
    hdrs=["clip_tokenizer.h"],
    deps=["@icu"],
)


cc_binary( 
    name="test",
    srcs=["test.cpp"],
    deps=[
        ":clip"
    ],
)

load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

pybind_extension(
    name = "clip_tokenizer_py",
    srcs = ["binding.cpp"],
    deps = [
        ":clip"
    ],
)

py_library(
    name = "clip_tokenizer_py",
    srcs = ["unit_test.py"],
    data = [
        ":clip_tokenizer_py.so"
    ],
)

py_binary(
    name = "unit_test",
    srcs = ["unit_test.py"],
    deps = [
        ":clip_tokenizer_py"
    ],
    data = [
        ":vocab.txt"
    ],
)