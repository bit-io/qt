#!/usr/bin/env sh
# Builds native/libqthsharp.a (the C++ part of qt-hsharp).
#
#   ./build.sh            Release build, native smoke test is run
#   ./build.sh Debug      Debug build
#   QTH_SKIP_TESTS=1 ./build.sh     build only
#
# Also what `bit` runs for this package: Bit.hk -> [build] native.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_type=${1:-Release}
build_dir="$here/build"

if ! command -v cmake >/dev/null 2>&1; then
    echo "qt-hsharp: cmake not found (Debian/Ubuntu: apt install cmake qt6-base-dev)" >&2
    exit 1
fi

if [ -n "${QTH_SKIP_TESTS:-}" ]; then tests=OFF; else tests=ON; fi

cmake -S "$here" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DQTH_BUILD_TESTS="$tests"
cmake --build "$build_dir" --parallel

if [ "$tests" = ON ]; then
    QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}" ctest --test-dir "$build_dir" --output-on-failure
fi

echo "qt-hsharp: built $here/libqthsharp.a"
