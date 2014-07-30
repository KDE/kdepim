<?xml version="1.0" encoding="utf-8"?>
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0"
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd">

  <kcfgfile name="kmail-mobilerc" />

  <group name="Appearance">
    <entry name="AppearanceShowHtmlStatusBar" type="Bool" />
    <entry name="AppearanceReplaceSmileys" type="Bool" />
    <entry name="AppearanceReduceQuotedFontSize" type="Bool" />
  </group>

  <group name="Composer">
    <entry name="ComposerInsertSignature" type="Bool" />
    <entry name="ComposertInsertSignatureAboveQuote" type="Bool" />
    <entry name="ComposerPrependSeparator" type="Bool" />
    <entry name="ComposerUseSmartQuoting" type="Bool" />
    <entry name="ComposerRequestMDN" type="Bool">
      <default>false</default>
    </entry>
    <entry name="ComposerUseRecentAddressCompletion" type="Bool" />
    <entry name="ComposerWordWrapAtColumn" type="Bool" />
    <entry name="ComposerWordWrapColumn" type="Int" />
    <entry name="ComposerReplaceReplyPrefixes" type="Bool" />
    <entry name="ComposerReplaceForwardPrefixes" type="Bool" />
    <entry name="ComposerOutlookCompatibleNaming" type="Bool" />
    <entry name="ComposerDetectMissingAttachments" type="Bool" />

    <entry name="ComposerTemplatesNewMessage" type="String" />
    <entry name="ComposerTemplatesReplyToSender" type="String" />
    <entry name="ComposerTemplatesReplyToAll" type="String" />
    <entry name="ComposerTemplatesForwardMessage" type="String" />

    <entry name="TooManyRecipients" type="Bool" key="too-many-recipients">
      <label>Warn if the number of recipients is larger than</label>
      <default>${WARN_TOOMANY_RECIPIENTS_DEFAULT}</default>
      <whatsthis>If the number of recipients is larger than this value, KMail Mobile will warn and ask for a confirmation before sending the mail. The warning can be turned off.</whatsthis>
    </entry>
    <entry name="RecipientThreshold" type="Int" key="recipient-threshold">
      <label></label>
      <default>5</default>
      <min>1</min>
      <max>100</max>
      <whatsthis>If the number of recipients is larger than this value, KMail Mobile will warn and ask for a confirmation before sending the mail. The warning can be turned off.</whatsthis>
    </entry>
  </group>

  <group name="Invitations">
    <entry name="InvitationsOutlookCompatible" type="Bool" />
    <entry name="InvitationsAutomaticSending" type="Bool" />
    <entry name="InvitationsDeleteAfterReply" type="Bool" />
  </group>

  <group name="Misc">
    <entry name="MiscEmptyTrashAtExit" type="Bool" />
    <entry name="MiscQuotaWarningThreshold" type="Int">
      <default>80</default>
    </entry>
    <entry name="MiscQuotaWarningColor" type="Color">
      <default>red</default>
    </entry>
  </group>

  <group name="MessageList">
    <entry name="MessageListSortingOption" type="Enum">
      <choices>
        <choice name="SortByDateTime" />
        <choice name="SortByDateTimeMostRecent" />
        <choice name="SortBySenderReceiver" />
        <choice name="SortBySubject" />
        <choice name="SortBySize" />
        <choice name="SortByActionItem" />
      </choices>
      <default>SortByDateTimeMostRecent</default>
    </entry>
    <entry name="MessageListSortingOrder" type="Enum">
      <choices>
        <choice name="Ascending" />
        <choice name="Descending" />
      </choices>
      <default>Ascending</default>
    </entry>
    <entry name="MessageListGroupingOption" type="Enum">
      <choices>
        <choice name="GroupByNone" />
        <choice name="GroupByDate" />
        <choice name="GroupBySenderReceiver" />
      </choices>
      <default>GroupByDate</default>
    </entry>
    <entry name="MessageListUseThreading" type="Bool">
      <default>true</default>
    </entry>
  </group>
  <group name="MDN">
    <entry name="notSendWhenEncrypted" type="Bool" key="not-send-when-encrypted">
      <label>Do not send MDNs in response to encrypted messages</label>
      <default>true</default>
    </entry>
    <entry name="MDNPolicy" type="Enum" key="default-policy">
      <label>Specifies the default policy to use, for the Message Disposition Notifications (for internal use only)</label>
      <choices>
        <choice name="Ignore">
          <label>Ignore</label>
        </choice>
        <choice name="Ask">
          <label>Ask</label>
        </choice>
        <choice name="Deny">
          <label>Deny</label>
        </choice>
        <choice name="AlwaysSend">
          <label>Always send</label>
        </choice>
      </choices>
      <default>Ignore</default>
    </entry>
    <entry name="MDNQuoteType" type="Enum" key="quote-message">
      <label>Specifies the default quoting action to take, when replying to a message (for internal use only)</label>
      <choices>
        <choice name="Nothing">
          <label>Nothing</label>
        </choice>
        <choice name="FullMessage">
          <label>Full message</label>
        </choice>
        <choice name="OnlyHeaders">
          <label>Only headers</label>
        </choice>
      </choices>
      <default>Nothing</default>
    </entry>
  </group>
</kcfg>
