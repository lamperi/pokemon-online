#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include "logmanager.h"
#include "../BattleManager/advancedbattledata.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battleinput.h"
#include "poketextedit.h"
#include "../Shared/battlecommands.h"
#include "../Utilities/coreclasses.h"

#ifdef QT5
#include <QApplication>
#include <QToolTip>
#endif

#ifdef QT5
#ifdef Q_OS_MACX
#include "mac/MacBattleSounds.h"
typedef MacBattleSounds BattleSoundsImpl;
#else
#include "qtbattlesounds.h"
typedef QtBattleSounds BattleSoundsImpl;
#endif Q_OS_MACX
#else
#include "qt4battlesounds.h"
typedef Qt4BattleSounds BattleSoundsImpl;
#endif

using namespace BattleCommands;

BaseBattleInfo::BaseBattleInfo(const PlayerInfo &me, const PlayerInfo &opp, int mode, int myself, int opponent)
    : myself(myself), opponent(opponent)
{
    this->mode =  mode;
    if (mode == ChallengeInfo::Doubles) {
        numberOfSlots = 4;
    } else if (mode == ChallengeInfo::Triples) {
        numberOfSlots = 6;
    } else {
        numberOfSlots = 2;
    }

    pInfo[myself] = me;
    pInfo[opponent] = opp;
}

BaseBattleWindow::BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf,
                                   int _ownid) : ignoreSpecs(NoIgnore)
{
    init(me, opponent, conf, _ownid);
}

void BaseBattleWindow::init(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &_conf,
                            int _ownid)
{
    ownid() = _ownid;
    conf() = _conf;
    conf().receivingMode[0] = this->conf().receivingMode[1] = BattleConfiguration::Spectator;
    conf().avatar[0] = me.avatar;
    conf().avatar[1] = opponent.avatar;
    conf().name[0] = me.name;
    conf().name[1] = opponent.name;

    myInfo = new BaseBattleInfo(me, opponent, conf().mode);
    info().gen = conf().gen;

    init();
    show();
}

BaseBattleWindow::BaseBattleWindow()
{
    ignoreSpecs=NoIgnore;
}

void BaseBattleWindow::delayNone() {
    delay();
}

void BaseBattleWindow::delay(qint64 msec)
{
    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        ptr->pause();
    }

    if (msec != 0)
        QTimer::singleShot(msec, this, SLOT(undelay()));
}

void BaseBattleWindow::undelay()
{
    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        ptr->unpause();
    }
}

void BaseBattleWindow::init()
{
    SpectatorWindow::init(conf());

    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        addOutput(ptr);
        ptr->deletable = false;
    }



    info().data = &data();

    setAttribute(Qt::WA_DeleteOnClose, true);
    QToolTip::setFont(QFont("Verdana",10));

    blankMessage = false;
    battleEnded = false;
    started() = false;

    QString title = tr("%1 vs %2").arg(data().name(0), data().name(1));
    log = LogManager::obj()->createLog(BattleLog, title);
    log->override = Log::OverrideNo; /* By default, no logging enabled */
    replay = LogManager::obj()->createLog(ReplayLog, title);
    replay->override = Log::OverrideNo;

    replayData.data = "battle_logs_v2\n";
    DataStream stream(&replayData.data, QIODevice::Append);
    stream << conf();
    replayData.t.start();

    setWindowTitle(tr("Battle between %1 and %2").arg(name(0), name(1)));

    QHBoxLayout *columns = new QHBoxLayout(this);
    columns->addLayout(mylayout = new QGridLayout());

    mylayout->addWidget(getSceneWidget(), 0, 0, 1, 3);
    QSettings settings;
    bool saveLog = settings.value("Battle/SaveLogs").toBool();
    mylayout->addWidget(saveLogs = new QCheckBox(tr("Save log")), 1, 0, 1, 2);
    saveLogs->setChecked(saveLog);
    mylayout->addWidget(musicOn = new QCheckBox(tr("Music")), 1, 1, 1, 2);
    mylayout->addWidget(flashWhenMoveDone = new QCheckBox(tr("Flash when a move is done")), 1, 2, 1, 2);

    QSettings s;
    musicOn->setChecked(s.value("BattleAudio/PlayMusic").toBool() || s.value("play_battle_cries").toBool());
    flashWhenMoveDone->setChecked(s.value("Battle/FlashOnMove").toBool());

    QVBoxLayout *chat = new QVBoxLayout();
    columns->addLayout(chat);
    chat->addWidget(mychat = getLogWidget());
    mychat->setAutoClear(false);
    chat->addWidget(myline = new QIRCLineEdit());
    QHBoxLayout * buttons = new QHBoxLayout();
    chat->addLayout(buttons);

    buttons->addWidget(mysend = new QPushButton(tr("C&hat")));
    buttons->addWidget(myclose = new QPushButton(tr("&Close")));
    buttons->addWidget(myignore = new QPushButton(tr("&Ignore spectators")));

    connect(myignore, SIGNAL(clicked()), SLOT(ignoreSpectators()));
    connect(myclose, SIGNAL(clicked()), SLOT(clickClose()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));

    loadSettings(this);

    battleSounds = new BattleSoundsImpl(this);
    //connect(battleSounds, &BattleSounds::soundsPlaying, this, &BaseBattleWindow::delayNone);
    //connect(battleSounds, &BattleSounds::soundsDone, this, &BaseBattleWindow::undelay);
    //connect(musicOn, &QCheckBox::toggled, battleSounds, &BattleSounds::toggleSounds);
    connect(battleSounds, SIGNAL(soundsPlaying()), SLOT(delay()));
    connect(battleSounds, SIGNAL(soundsDone()), SLOT(undelay()));
    connect(musicOn, SIGNAL(toggled(bool)), battleSounds, SLOT(toggleSounds(bool)));

    QObject *ptr2 = dynamic_cast<QObject*>(getBattle());

    if (ptr2) {
        connect(ptr2, SIGNAL(playCry(int)), battleSounds, SLOT(playCry(int)));
    }

    battleSounds->toggleSounds(musicOn->isChecked());
}

bool BaseBattleWindow::flashWhenMoved() const
{
    return flashWhenMoveDone->isChecked();
}

void BaseBattleWindow::changeCryVolume(int v)
{
    battleSounds->setCryVolume(v);
}

void BaseBattleWindow::changeMusicVolume(int v)
{
    battleSounds->setMusicVolume(v);
}

int BaseBattleWindow::player(int spot) const
{
    return spot % 2;
}

int BaseBattleWindow::opponent(int player) const
{
    return !player;
}

QString BaseBattleWindow::name(int spot) const
{
    return data().name(spot);
}

void BaseBattleWindow::checkAndSaveLog()
{
    if (!log) {
        return;
    }
    log->pushList(getLog()->getLog());
    log->pushHtml("</body>");
    replay->setBinary(replayData.data);
    if (saveLogs->isChecked()) {
        log->override = Log::OverrideYes;
        replay->override = Log::OverrideYes;
    }

    log->close();
    log = NULL;
    replay->close();
    replay = NULL;
}

void BaseBattleWindow::closeEvent(QCloseEvent *)
{
    checkAndSaveLog();
    emit closedBW(battleId());
    close();
}

void BaseBattleWindow::close()
{
    writeSettings(this);
    QWidget::close();
}

void BaseBattleWindow::disable()
{
    mysend->setDisabled(true);
    myline->setDisabled(true); 
    checkAndSaveLog();

    getInput()->entryPoint(BattleEnum::BlankMessage);
    auto mess = std::shared_ptr<QString>(new QString(toBoldColor(tr("The window was disabled due to one of the players closing the battle window."), Qt::blue)));
    getInput()->entryPoint(BattleEnum::PrintHtml, &mess);
}

void BaseBattleWindow::clickClose()
{
    emit closedBW(battleId());
    close();
    return;
}

void BaseBattleWindow::sendMessage()
{
    QString message = myline->text();

    if (message.size() != 0) {
        emit battleMessage(battleId(), message);
        myline->clear();
    }
}

void BaseBattleWindow::receiveData(const QByteArray &inf)
{
    if (inf[0] == char(SpectatorChat) && ignoreSpecs != NoIgnore) {
        return;
    }
    if ( (inf[0] == char(BattleChat) || inf[0] == char(EndMessage)) && ignoreSpecs == char(IgnoreAll)) {
        if (inf[1] == char(info().opponent)) {
            return;
        }
    }

    addReplayData(inf);

    SpectatorWindow::receiveData(inf);
}

void BaseBattleWindow::addReplayData(const QByteArray &inf)
{
    DataStream stream(&replayData.data, QIODevice::Append);
    stream << quint32(replayData.t.elapsed()) << inf;
}

void BaseBattleWindow::ignoreSpectators()
{
    ignoreSpecs = ignoreSpecs +1;
    if (ignoreSpecs > IgnoreAll) {
        ignoreSpecs = 0;
    }

    switch (ignoreSpecs) {
    case NoIgnore:
        myignore->setText(tr("&Ignore spectators")); break;
    case IgnoreSpecs:
        myignore->setText(tr("&Ignore everybody")); break;
    case IgnoreAll:
        myignore->setText(tr("Stop &ignoring")); break;
    }
}

void BaseBattleWindow::onSendOut(int, int, ShallowBattlePoke *, bool)
{
    flashIfNeeded();
}

void BaseBattleWindow::onSendBack(int spot, bool)
{
    flashIfNeeded();

    switchToNaught(spot);
}

void BaseBattleWindow::onUseAttack(int, int, bool)
{
    flashIfNeeded();
}

void BaseBattleWindow::flashIfNeeded()
{
    if(!this->window()->isActiveWindow() && flashWhenMoved()) {
        qobject_cast<QApplication*>(qApp)->alert(this, 0);
    }
}

void BaseBattleWindow::onDisconnection()
{
    mychat->insertHtml("<br><i><b>Disconnected from Server!</b></i>");
}

void BaseBattleWindow::onKo(int spot)
{
    switchToNaught(spot);
}

void BaseBattleWindow::onSpectatorJoin(int id, const QString &n)
{
    addSpectator(true, id, n);
}

void BaseBattleWindow::onSpectatorLeave(int id)
{
    addSpectator(false, id);
}

void BaseBattleWindow::onBattleEnd(int, int)
{
    battleEnded = true;
}

void BaseBattleWindow::addSpectator(bool come, int id, const QString &)
{
    if (come) {
        spectators.insert(id);
    } else {
        spectators.remove(id);
    }
}

