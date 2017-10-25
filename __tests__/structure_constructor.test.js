const { load, Gtk} = require('../');
const GLib = load('GLib');
const GObject = load('GObject');

describe('Structure constructor', function() {
    describe('with properties', function() {
        it('GLib.MainLoop', function() {
            const loop = new GLib.MainLoop(null, false);
            expect(typeof(loop)).toEqual('object');
            const running = loop.isRunning();
            expect(running).toBe(false);
        });
    });

    describe('without properties', function() {
        it('GLib.MainContext', function() {
            const context = new GLib.MainContext();
            expect(typeof(context)).toEqual('object');
        });
    });
});
