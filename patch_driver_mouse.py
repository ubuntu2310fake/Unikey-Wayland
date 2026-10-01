import re

with open("ring0-engine/ukw_driver.c", "r") as f:
    code = f.read()

# Add mouse to ukw_ids
old_ids = """static const struct input_device_id ukw_ids[] = {
    { .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
      .evbit = { BIT_MASK(EV_KEY) },
      .keybit = { [BIT_WORD(KEY_A)] = BIT_MASK(KEY_A) } },
    { },
};"""

new_ids = """static const struct input_device_id ukw_ids[] = {
    { .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
      .evbit = { BIT_MASK(EV_KEY) },
      .keybit = { [BIT_WORD(KEY_A)] = BIT_MASK(KEY_A) } },
    { .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
      .evbit = { BIT_MASK(EV_KEY) },
      .keybit = { [BIT_WORD(BTN_LEFT)] = BIT_MASK(BTN_LEFT) } },
    { },
};"""
code = code.replace(old_ids, new_ids)

with open("ring0-engine/ukw_driver.c", "w") as f:
    f.write(code)
