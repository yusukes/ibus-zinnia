#include <ibus.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "zinnia_component.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
    { "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    g_debug ("bus disconnected");
    ibus_quit ();
}


static void
start_component (void)
{
    GList *engines, *p;
    IBusComponent *component;

    ibus_init ();

    bus = ibus_bus_new ();
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

    component = ibus_zinnia_get_component ();

    factory = ibus_factory_new (ibus_bus_get_connection (bus));

    engines = ibus_component_get_engines (component);
    for (p = engines; p != NULL; p = p->next) {
        IBusEngineDesc *engine = (IBusEngineDesc *)p->data;
        ibus_factory_add_engine (factory,
				 ibus_engine_desc_get_name(engine),
				 IBUS_TYPE_ZINNIA_ENGINE);
    }

    if (ibus) {
        ibus_bus_request_name (bus, "com.google.IBus.Zinnia", 0);
    }
    else {
        ibus_bus_register_component (bus, component);
    }

    g_object_unref (component);

    ibus_main ();
}

int
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("- ibus zinnia engine component");

    g_option_context_add_main_entries (context, entries, "ibus-zinnia");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    start_component ();
    return 0;
}
