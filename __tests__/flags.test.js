const { Gtk } = require('../');


describe('flags should be objects on the owning namespace object', () => {
  test('flags exist on the owning namespace object', () => {
    expect(Gtk).toHaveProperty('AccelFlags');
    expect(Gtk.AccelFlags).not.toBeUndefined();
  });

  test('flag keys are uppercase strings with number values', () => {
    expect(Gtk.AccelFlags).toHaveProperty('LOCKED');
    expect(Gtk.AccelFlags.LOCKED).not.toBeUndefined();
    expect(typeof (Gtk.AccelFlags.LOCKED)).toEqual('number');
  });

  test('flag object should match a known value from the gtk docs', () => {
    // from https://lazka.github.io/pgi-docs/Gtk-3.0/flags.html#Gtk.DebugFlag
    expect(Gtk.DebugFlag).toEqual({
      MISC: 1,
      PRINTING: 1024,
      RESIZE: 1048576,
      MODULES: 128,
      INTERACTIVE: 131072,
      UPDATES: 16,
      BASELINES: 16384,
      PLUGSOCKET: 2,
      BUILDER: 2048,
      LAYOUT: 2097152,
      GEOMETRY: 256,
      TOUCHSCREEN: 262144,
      KEYBINDINGS: 32,
      PIXEL_CACHE: 32768,
      TEXT: 4,
      SIZE_REQUEST: 4096,
      ICONTHEME: 512,
      ACTIONS: 524288,
      MULTIHEAD: 64,
      NO_PIXEL_CACHE: 65536,
      TREE: 8,
      NO_CSS_CACHE: 8192
    });
  });
});
