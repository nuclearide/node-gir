const { load, Gtk } = require('../');
const GdkPixbuf = load('GdkPixbuf');

const pixbuf = new GdkPixbuf.Pixbuf(0, false, 1, 1, 1);
const win = new Gtk.Window({
    type: Gtk.WindowType.toplevel,
    title: "Node.JS GTK Window"
});

describe('Gtk.Object', function() {
    describe('property', function() {
        describe('boolean', function() {
            it('should be true', function() {
                win.modal = true;
                expect(win.modal).toBe(true);
            });

            it('should be false', function() {
                win.modal = false;
                expect(win.modal).toBe(false);
            });
        });

        describe('string', function() {
            it('set Lancelot', function() {
                win.title = 'Lancelot';
                expect(win.title).toEqual('Lancelot');
            });

            it('get Lancelot', function() {
                expect(win.title).toEqual('Lancelot');
                expect(win.title).not.toEqual('');
                expect(win.title).not.toEqual(' ');
            });
        });

        describe('integer', function() {
            it('set 1', function() {
                win['default-height'] = 1;
                expect(win['default-height']).toEqual(1);
            });

            it('get 1', function() {
                expect(win['default-height']).toEqual(1);
                expect(win['default-height']).not.toEqual(0);
                expect(win['default-height']).not.toEqual(-1);
            });
        });

        describe('double', function() {
            it('set 0.33', function() {
                win.opacity = 0.33;
                expect(win.opacity).toEqual(0.33);
            });

            it('get 0.33', function() {
                expect(win.opacity).toEqual(0.33);
                expect(win.opacity).not.toEqual(0.001);
                expect(win.opacity).not.toEqual(2);
                expect(win.opacity).not.toEqual(1.23);
            });
        });

        describe('object', function() {
            it('set icon', function() {
                win.icon = pixbuf;
            });

            it('get icon', function() {
                expect(typeof(win.icon)).toEqual('object');
                expect(win.icon).not.toEqual(win);
            });
        });
    });
});
