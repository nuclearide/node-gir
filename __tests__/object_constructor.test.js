const { load, Gtk } = require('../');

const GLib = load('GLib');
const GObject = load('GObject');

describe('Object constructor', function() {
    describe('with "new" operator', function() {
        it('without arguments', function() {
            const label = new Gtk.Label();
            expect(label.label).toEqual('');
        });

        it('with properties', function() {
            const label = new Gtk.Label({ label: 'aText99'});
            expect(label.label).toEqual('aText99');
        });
    });
});
