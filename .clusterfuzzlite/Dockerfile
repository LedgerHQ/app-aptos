FROM ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder-lite:latest AS LITE_BUILDER

# Base image with clang toolchain
FROM gcr.io/oss-fuzz-base/base-builder:v1

# Copy the project's source code.
COPY . $SRC/app-aptos
COPY --from=LITE_BUILDER /opt/ledger-secure-sdk $SRC/app-aptos/BOLOS_SDK

# Working directory for build.sh
WORKDIR $SRC/app-aptos

# Copy build.sh into $SRC dir.
COPY ./.clusterfuzzlite/build.sh $SRC/