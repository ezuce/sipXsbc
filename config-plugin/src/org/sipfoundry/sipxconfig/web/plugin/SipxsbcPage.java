/*
 *
 * Copyright (C) 2012 eZuce Inc.
 *
 * $
 */
package org.sipfoundry.sipxconfig.web.plugin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.sipxsbc.Sipxsbc;
import org.sipfoundry.sipxconfig.sipxsbc.SipxsbcSettings;

public abstract class SipxsbcPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "plugin/SipxsbcPage";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxsbcSettings getSettings();

    public abstract void setSettings(SipxsbcSettings settings);

    @InjectObject("spring:sipxsbc")
    public abstract Sipxsbc getSipxsbc();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getSipxsbc().getSettings());
        }
    }

    public void apply() {
        getSipxsbc().saveSettings(getSettings());
    }

}
