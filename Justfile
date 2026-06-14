set shell := ["zsh", "-uc"]

default:
    @just --list

status:
    git status --short --branch
    git log -1 --oneline

version:
    rg -n 'ARDUCON_FIRMWARE_VERSION|PRODUCT_NAME_LONG_TXT' Software/AtmelStudio7/Arducon/Arducon/EepromManager.h

cli-build:
    pwsh -NoProfile -File ./build-cli-release.ps1 -Clean

interrupt-regression:
    node ./scripts/check-firmware-interrupts.mjs

host-unit-tests:
    pwsh -NoProfile -File ./run-host-unit-tests.ps1

test: host-unit-tests interrupt-regression

serial-bootloader-smoke port:
    pwsh -NoProfile -File ./test-bootloader-serial.ps1 -Port "{{port}}"

serial-bootloader-smoke-via-app port:
    pwsh -NoProfile -File ./test-bootloader-serial.ps1 -Port "{{port}}" -RequestBootloaderFromApp

serial-update port hex="tmp/cli-release/Arducon.hex":
    ${AVRDUDE:-avrdude} -v -p m328p -c arduino -P "{{port}}" -b 115200 -D -U "flash:w:{{hex}}:i"

flash-latest port="/dev/cu.usbserial-AB9PEG0Z":
    just package-skip-build
    pwsh -NoProfile -File ./flash-arducon-usb.ps1 -Port "{{port}}"

stop-running-arducon port="/dev/cu.usbserial-AB9PEG0Z":
    pwsh -NoProfile -File ./flash-arducon-usb.ps1 -Port "{{port}}" -SkipUpdate

size:
    pwsh -NoProfile -File ./check-firmware-size.ps1 -Configuration Release -HexPath ./tmp/cli-release/Arducon.hex

package:
    pwsh -NoProfile -File ./build-release-package.ps1

package-skip-build:
    pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild

validate:
    pwsh -NoProfile -File ./validate-release-package.ps1

validate-package package_dir:
    pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir "{{package_dir}}"

checklist checklist phase:
    node ./scripts/check-release-checklist.mjs --file "{{checklist}}" --phase "{{phase}}"

pre-tag checklist:
    node ./scripts/check-release-checklist.mjs --file "{{checklist}}" --phase pre-tag

final-check checklist:
    node ./scripts/check-release-checklist.mjs --file "{{checklist}}" --phase final

tag version:
    git tag "v{{version}}"
    git push origin "v{{version}}"

github-release version:
    gh release create "v{{version}}" \
      "./release-packages/Arducon-v{{version}}/Arducon-Update-v{{version}}-ATmega328P.hex" \
      "./release-packages/Arducon-v{{version}}/Arducon-v{{version}}-ATmega328P-Release-Files.zip" \
      --repo OpenARDF/Arducon \
      --title "Arducon v{{version}}" \
      --notes-file "./release-notes-v{{version}}.md"

release-view version:
    gh release view "v{{version}}" --repo OpenARDF/Arducon
