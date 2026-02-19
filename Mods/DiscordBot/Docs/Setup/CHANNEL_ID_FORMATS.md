# Channel ID Format Test Examples

This file demonstrates all supported channel ID formats and combinations.

## Format 1: Single Channel (Comma-separated)
```ini
[DiscordBot]
ChatChannelId=1234567890123456789
```
**Result**: 1 channel loaded

## Format 2: Multiple Channels (Comma-separated)
```ini
[DiscordBot]
ChatChannelId=1234567890123456789,9876543210987654321,5555555555555555555
```
**Result**: 3 channels loaded

## Format 3: Multiple Channels with Spaces (Comma-separated)
```ini
[DiscordBot]
ChatChannelId=1234567890123456789, 9876543210987654321 , 5555555555555555555
```
**Result**: 3 channels loaded (whitespace automatically trimmed)

## Format 4: Single Channel (Array format - backward compatible)
```ini
[DiscordBot]
+ChatChannelId=1234567890123456789
```
**Result**: 1 channel loaded

## Format 5: Multiple Channels (Array format - backward compatible)
```ini
[DiscordBot]
+ChatChannelId=1234567890123456789
+ChatChannelId=9876543210987654321
+ChatChannelId=5555555555555555555
```
**Result**: 3 channels loaded

## Format 6: Mixed Format (Both comma-separated and array)
```ini
[DiscordBot]
ChatChannelId=1234567890123456789,9876543210987654321
+ChatChannelId=5555555555555555555
+ChatChannelId=7777777777777777777
```
**Result**: 4 channels loaded (merged from both formats)

## Format 7: Mixed with Duplicates (Automatically handled)
```ini
[DiscordBot]
ChatChannelId=1234567890123456789,9876543210987654321
+ChatChannelId=1234567890123456789
+ChatChannelId=5555555555555555555
```
**Result**: 3 channels loaded (duplicate 1234567890123456789 is not added twice)

## Implementation Notes

The channel loading logic works as follows:

1. **First Pass**: Load comma-separated format
   - Reads `ChatChannelId` as a single string
   - Splits by comma
   - Trims whitespace from each ID
   - Adds non-empty IDs to the channel list

2. **Second Pass**: Load array format
   - Reads all `+ChatChannelId` entries
   - Checks each ID against existing list
   - Only adds if not already present
   - Ensures no duplicates

3. **Result**: Both formats can coexist, and duplicates are automatically eliminated

## Benefits

- **Simpler**: For multiple channels, one line is easier than many
- **Backward Compatible**: Existing configs with `+ChatChannelId` still work
- **Flexible**: Mix both formats as needed
- **Safe**: Whitespace handling and duplicate prevention built-in
