const { load, Gtk } = require('../');

const GIRepository = load('GIRepository');
const repo = new GIRepository.Repository();
repo.require('Gtk', '3.0', 0);

describe('enums should be objects on the owning namespace object', () => {
  test('enums exist on the owning namespace object', () => {
    expect(Gtk).toHaveProperty('WindowType');
    expect(Gtk.WindowType).not.toBeUndefined();
  });

  test('enum keys are uppercase strings with number values', () => {
    expect(Gtk.WindowType).toHaveProperty('TOPLEVEL');
    expect(Gtk.WindowType.TOPLEVEL).not.toBeUndefined();
    expect(typeof (Gtk.WindowType.TOPLEVEL)).toEqual('number');
  });

  test('functions can return enum values', () => {
    const info = repo.findByName('Gtk', 'Window');
    const type = info.getType();
    expect(type).not.toBe(null);
    expect(type).not.toBe(undefined);
    expect(typeof (type)).toEqual('number');
  });
});
