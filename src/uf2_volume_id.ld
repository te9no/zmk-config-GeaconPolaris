/* XIAO nRF52840: place the identity immediately after the 256-byte vectors.
 * An incompatible vector layout must fail at link time. */
ASSERT(. - __rom_start_address <= 0x100, "UF2 identity overlaps vectors");
. = . + (0x100 - (. - __rom_start_address));
LONG(0x31444956) /* VID1 */
LONG(0x535a424b) /* KBZS */
#if defined(CONFIG_SHIELD_Polaris_L_Base)
LONG(0x034c)
#elif defined(CONFIG_SHIELD_Polaris_R_Base)
LONG(0x0352)
#else
#error Missing UF2 volume target
#endif
