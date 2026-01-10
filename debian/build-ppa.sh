#!/bin/bash
set -e

PACKAGE_NAME="libcuid2"
VERSION="1.0.1"
REVISION="1ubuntu4"
RELEASES=("noble" "jammy")
BUILD_AREA="../build-area"
GPG_KEY="${GPG_KEY:-}"
PPA_NAME=""

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_prerequisites() {
    info "Checking prerequisites..."

    local missing_tools=()

    for tool in debuild dch git tar; do
        if ! command -v "$tool" &> /dev/null; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -ne 0 ]; then
        error "Missing required tools: ${missing_tools[*]}"
        info "Install with: sudo apt-get install devscripts debhelper git"
        exit 1
    fi

    success "All prerequisites satisfied"
}

clean_build_area() {
    info "Cleaning build area..."
    mkdir -p "$BUILD_AREA"
    rm -rf "${BUILD_AREA}/${PACKAGE_NAME}_${VERSION}"*
    success "Build area cleaned"
}

create_orig_tarball() {
    info "Creating orig tarball..."

    local orig_tarball="${BUILD_AREA}/${PACKAGE_NAME}_${VERSION}.orig.tar.xz"

    if [ -f "$orig_tarball" ]; then
        warning "Orig tarball already exists: $orig_tarball"
        return
    fi

    git archive --format=tar --prefix="${PACKAGE_NAME}-${VERSION}/" HEAD | \
        xz -9 > "$orig_tarball"

    success "Created orig tarball: $orig_tarball"
}

build_for_release() {
    local release="$1"
    local release_version="${VERSION}-${REVISION}~${release}1"

    info "Building source package for Ubuntu ${release}..."

    local work_dir="${BUILD_AREA}/${PACKAGE_NAME}-${VERSION}"
    rm -rf "$work_dir"
    mkdir -p "$work_dir"

    tar -xf "${BUILD_AREA}/${PACKAGE_NAME}_${VERSION}.orig.tar.xz" -C "$BUILD_AREA"
    cp -r debian "$work_dir/"
    cd "$work_dir"

    # Update changelog with release-specific version at the top
    info "Updating changelog for ${release}..."
    DEBFULLNAME="Visus Development Team" \
    DEBEMAIL="admin@projects.visus.io" \
    dch --force-distribution --distribution "${release}" \
        --newversion "${release_version}" \
        "Rebuild for Ubuntu ${release}"

    info "Running debuild for ${release}..."

    if [ -n "$GPG_KEY" ]; then
        debuild -S -sa -k"$GPG_KEY"
    else
        debuild -S -sa -us -uc
    fi

    cd - > /dev/null

    success "Built source package for ${release}: ${PACKAGE_NAME}_${release_version}"
}

show_summary() {
    info "Build Summary"
    echo "=============================================="
    echo "Package: ${PACKAGE_NAME}"
    echo "Version: ${VERSION}"
    echo "Build area: ${BUILD_AREA}"
    echo ""
    echo "Generated files:"
    ls -lh "${BUILD_AREA}"/*.{dsc,changes,tar.xz,buildinfo} 2>/dev/null || true
    echo ""

    if [ -z "$GPG_KEY" ]; then
        warning "Packages are UNSIGNED"
        info "To sign packages, run:"
        info "  debsign ${BUILD_AREA}/${PACKAGE_NAME}_*.changes"
        echo ""
    fi

    if [ -n "$PPA_NAME" ]; then
        info "To upload to PPA:"
        echo "  dput ppa:${PPA_NAME} ${BUILD_AREA}/${PACKAGE_NAME}_*~noble1_source.changes"
        echo "  dput ppa:${PPA_NAME} ${BUILD_AREA}/${PACKAGE_NAME}_*~jammy1_source.changes"
    else
        info "To upload to PPA (specify with -p option):"
        echo "  dput ppa:<your-ppa-name> ${BUILD_AREA}/${PACKAGE_NAME}_*~noble1_source.changes"
        echo "  dput ppa:<your-ppa-name> ${BUILD_AREA}/${PACKAGE_NAME}_*~jammy1_source.changes"
    fi
}

main() {
    info "Starting PPA build process for ${PACKAGE_NAME} ${VERSION}"

    while [[ $# -gt 0 ]]; do
        case $1 in
            -k|--key)
                GPG_KEY="$2"
                shift 2
                ;;
            -p|--ppa)
                PPA_NAME="$2"
                shift 2
                ;;
            -h|--help)
                echo "Usage: $0 [-k GPG_KEY] [-p PPA_NAME] [-h]"
                echo ""
                echo "Options:"
                echo "  -k, --key GPG_KEY    GPG key ID for signing packages"
                echo "  -p, --ppa PPA_NAME   PPA name (e.g., username/ppa-name)"
                echo "  -h, --help           Show this help message"
                exit 0
                ;;
            *)
                error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    check_prerequisites
    clean_build_area
    create_orig_tarball

    for release in "${RELEASES[@]}"; do
        build_for_release "$release"
    done

    show_summary
    success "PPA build process completed successfully!"
}

main "$@"
