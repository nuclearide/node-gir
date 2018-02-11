const { Gtk } = require('../');

describe('Object constructor', () => {
  describe('with "new" operator', () => {
    it('without arguments', () => {
      const label = new Gtk.Label();
      expect(label.label).toEqual('');
    });

    it('with properties', () => {
      const label = new Gtk.Label({ label: 'aText99' });
      expect(label.label).toEqual('aText99');
    });
  });
});
