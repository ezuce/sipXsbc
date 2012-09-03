//
// This is an example of routing calls to a conference bridge
//

function conference_is_routable(profile)
{
    if (profile.sipMessage.getRequestUriUser() == "1000")
    {
        profile.bridgeToConference("1000");
        //
        // You can set a PIN for the conference by entring a second parameter
        // profile.bridgeToConference("1000", "9999");
        // 9999 will be the PIN asked for every participant that joins conference 1000
        //
        return true;
    }
    return false;
}