"""Audit generated Polaris ESB relay builds; run inside the workspace dev image."""
import hashlib
import sys
import subprocess
from pathlib import Path

import yaml

profile = Path('/zmk-workspace/.zmk-workspace/profiles/polaris-esb-validation')
repo = Path('/zmk-workspace/work/polaris-esb')
sys.path.insert(0, str(profile / 'west/zephyr/scripts/dts/python-devicetree/src'))
from devicetree import dtlib

checks = 0


def check(ok, message):
    global checks
    assert ok, message
    checks += 1


def enabled(node):
    while node is not None:
        if 'status' in node.props and node.props['status'].to_string() != 'okay':
            return False
        node = node.parent
    return True


def walk(node):
    yield node
    for child in node.nodes.values():
        yield from walk(child)


targets = yaml.safe_load((repo / 'build.yaml').read_text())['include']
check(len(targets) == 2, 'ESB-only matrix')
pins = {
    'zmk': 'e5c9b6915b56801193e359dd9bad4a167ce0d1b8',
    'sdk-nrf': '9b3d2623fdcd9c0fd0284f860beea924568c9826',
    'nrfxlib': 'dfadf17305d8f000eda9aa74a5b9ff1c5647a23e',
}
projects = yaml.safe_load((repo / 'config/west.yml').read_text())['manifest']['projects']
for name, sha in pins.items():
    check(next(p for p in projects if p['name'] == name)['revision'] == sha, name + ' SHA')

module = next(p for p in projects if p['name'] == 'zmk-feature-split-esb')
check(module['url'] == 'https://github.com/te9no/zmk-feature-split-esb', 'maintained fork URL')
check(len(module['revision']) == 40 and all(c in '0123456789abcdef' for c in module['revision']), 'ESB full immutable SHA')
pins['zmk-feature-split-esb'] = module['revision']
module_dir = profile / 'west/zmk-feature-split-esb'
check(subprocess.check_output(['git', '-C', str(module_dir), 'rev-parse', 'HEAD'], text=True).strip() == pins['zmk-feature-split-esb'], 'actual compiled module SHA')
check(not subprocess.check_output(['git', '-C', str(module_dir), 'status', '--porcelain'], text=True).strip(), 'compiled module checkout clean')

for target in targets:
    name = target['artifact-name']
    left = name.startswith('Polaris_L_')
    directory = profile / 'build' / name / 'zephyr'
    d = dtlib.DT(str(directory / 'zephyr.dts'))
    config = dict(line.split('=', 1) for line in (directory / '.config').read_text().splitlines()
                  if line.startswith('CONFIG_'))

    def c(symbol, value='y'):
        check(config.get('CONFIG_' + symbol, 'n') == value, name + ' ' + symbol)

    c('ZMK_SPLIT_ESB')
    c('ZMK_SPLIT_ROLE_CENTRAL', 'y' if left else 'n')
    for symbol in ('ZMK_BLE', 'ZMK_SPLIT_BLE', 'BT',
                   'ZMK_BLE_MANAGEMENT', 'ZMK_WATCHDOG_SPLIT_RELAY'):
        c(symbol, 'n')
    for symbol in ('ZMK_SPLIT_RELAY_EVENT', 'ZMK_SPLIT_ESB_RELAY_EVENT',
                   'ZMK_PMW3610_SPLIT_RPC_RELAY', 'ZMK_PMW3610_PROTOBUF',
                   'ZMK_CUSTOM_SETTINGS_SPLIT_RPC_RELAY', 'ZMK_SETTINGS_RPC'):
        c(symbol)
    c('ZMK_SPLIT_RELAY_EVENT_DATA_LEN', '240')
    c('ZMK_PMW3610_STUDIO_RPC', 'y' if left else 'n')
    c('ZMK_CUSTOM_SETTINGS_STUDIO_RPC', 'y' if left else 'n')
    c('ZMK_SETTINGS_RPC_STUDIO', 'y' if left else 'n')
    c('ZMK_USB', 'y' if left else 'n')
    for symbol in ('USB_DEVICE_STACK', 'ZMK_USB_LOGGING', 'LOG_BACKEND_UART',
                   'ZMK_CDC_ACM_BOOTLOADER_TRIGGER', 'ZMK_BATTERY_REPORTING',
                   'ZMK_SPLIT_ESB_PROTO_TX_ACK', 'ZMK_SPLIT_ESB_MSG_POSTFIX_CRC'):
        c(symbol)
    c('ZMK_SPLIT_ESB_PERIPHERAL_ID', '0')
    c('ZMK_SPLIT_ESB_PERIPHERAL_COUNT', '1')
    c('ESB_MAX_PAYLOAD_LENGTH', '48')
    c('LOG_MAX_LEVEL', '3')
    c('LOG_OVERRIDE_LEVEL', '0')
    c('ZMK_SPLIT_ESB_AUTO_HEAL_KEY_POS_MAX', '56')
    c('SPI_THREE_WIRE_GPIO_LOG_LEVEL', '2') if not left else None

    for label in ('xiao_serial', 'xiao_spi'):
        check(not enabled(d.label2node[label]), name + ' released ' + label)
    check(enabled(d.label2node['xiao_i2c']) == left, name + ' I2C ownership')

    battery = d.label2node['vbatt']
    check(int.from_bytes(battery.props['io-channels'].value[-4:], 'big') == 0, name + ' battery A0')
    check('power-gpios' not in battery.props, name + ' battery no board power GPIO')
    check(battery.props['output-ohms'].to_num() == 470000, name + ' divider output')
    check(battery.props['full-ohms'].to_num() == 1470000, name + ' divider total')
    check(battery.props['mv-to-pct-thresholds'].to_nums() ==
          [1100, 1150, 1200, 1220, 1240, 1260, 1280, 1300, 1320, 1350, 1400], name + ' NiMH')

    esb = d.get_node('/esb_split')
    check(esb.props['base-addr-0'].to_nums() == [0x50, 0x4f, 0x4c, 0x41], name + ' address 0')
    check(esb.props['base-addr-1'].to_nums() == [0x52, 0x49, 0x53, 0x27], name + ' address 1')
    check(esb.props['addr-prefix'].to_nums() == [0x91, 0x26, 0x53, 0xa4, 0x75, 0xc6, 0x37, 0xe8], name + ' prefixes')

    cdc = [n for n in walk(d.root) if 'compatible' in n.props and
           'zephyr,cdc-acm-uart' in n.props['compatible'].to_strings() and enabled(n)]
    check(len(cdc) == (2 if left else 1), name + ' active CDC count')
    console = d.get_node('/chosen').props['zephyr,console'].to_path()
    check(console is d.label2node['board_cdc_acm_uart' if left else 'snippet_zmk_usb_logging_uart'], name + ' debug CDC chosen')
    trigger = d.label2node['bootloader_trigger'].props['cdc-port'].to_node()
    check(trigger is d.label2node['snippet_studio_rpc_usb_uart' if left else 'snippet_zmk_usb_logging_uart'], name + ' boot trigger CDC')

    if left:
        for symbol in ('ZMK_DISPLAY_STATUS_SCREEN_BUILT_IN', 'ZMK_STUDIO_RPC',
                       'ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING',
                       'ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY',
                       'DYA_ANALOG_INPUT', 'ZMK_BATTERY_VOLTAGE_DIVIDER_OVERSAMPLING',
                       'RGBLED_WIDGET_BATTERY_SHOW_SELF'):
            c(symbol)
        c('RGBLED_WIDGET_BATTERY_SHOW_PERIPHERALS', 'n')
        c('NICE_OLED', 'n')
        check(battery.props['compatible'].to_string() == 'te9no,battery-voltage-divider-oversampling', name + ' oversampling node')
        analog = d.label2node['anin0']
        check([int.from_bytes(analog.nodes[axis].props['io-channels'].value[-4:], 'big')
               for axis in ('x-ch', 'y-ch')] == [2, 3], name + ' JOY ADC2/3')
        check(analog.props['sampling-hz'].to_num() == 100 and analog.props['report-interval-ms'].to_num() == 8, name + ' JOY timing')
    else:
        c('PMW3610')
        c('SPI_THREE_WIRE_GPIO')
        check(enabled(d.label2node['trackball']), name + ' PMW enabled')
        check(d.label2node['trackball'].props['cpi'].to_num() == 800, name + ' CPI800')
        check('disable-burst-read' in d.label2node['trackball'].props, name + ' nonburst preserved')

    # Inspect the linked ELF, not just requested Kconfig flags.
    cache_lines = (directory.parent / 'CMakeCache.txt').read_text().splitlines()
    nm = next(line.split('=', 1)[1] for line in cache_lines
              if line.startswith('CMAKE_NM:FILEPATH='))
    symbols = subprocess.check_output([nm, str(directory / 'zmk.elf')], text=True)
    for symbol in ('zmk_split_central_send_relay_event' if left else 'on_pmw3610_relay_request',):
        check(symbol in symbols, name + ' linked ' + symbol)

    binary = (directory / 'zmk.bin').read_bytes()
    check(b'ESB relay_events=%ld received' in binary, name + ' INFO relay count present')
    for debug_literal in (b'layer_id: %d position: %d, binding name: %s',
                          b'Pending save for layer %d at key position %d:',
                          b'Mouse movement set to %d/%d',
                          b'Modifier %d count %d'):
        check(debug_literal not in binary, name + ' DBG format absent: ' + str(debug_literal))
    if left:
        check(b'ESB source=0 position_events=%ld received' in binary, name + ' INFO count present')
        check(b'ESB source=0 battery=%u%% received' in binary, name + ' INFO battery present')
    else:
        check(b'PMW3610 async init step %d' in binary, name + ' PMW INFO init present')

    uf2 = directory / 'zmk.uf2'
    exported = Path('/zmk-workspace/firmware/zmk-config-GeaconPolaris/codex-zmk-0.4-esb-validation') / (name + '.uf2')
    check(uf2.read_bytes() == exported.read_bytes(), name + ' exported UF2 exact')
    print(f'PASS {name}: CDC={[n.name for n in cdc]}; {uf2.stat().st_size} bytes; SHA256={hashlib.sha256(uf2.read_bytes()).hexdigest()}')

print(f'PASS: {checks} assertions, ESB relay 2-target DTS/config/source/output gate. Hardware and DYA RPC round-trip pending.')
