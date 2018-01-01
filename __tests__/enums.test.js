const { Gtk } = require('../');

describe('enums should be objects on the owning namespace object', () => {
  test('enums exist on the owning namespace object', () => {
    expect(Gtk).toHaveProperty('WindowType');
    expect(Gtk.WindowType).not.toBeUndefined();
  });

  test('enum keys are uppercase strings with number values', () => {
    expect(Gtk.WindowType).toHaveProperty('TOPLEVEL');
    expect(Gtk.WindowType.TOPLEVEL).not.toBeUndefined();
    expect(typeof(Gtk.WindowType.TOPLEVEL)).toEqual('number');
  });
});
