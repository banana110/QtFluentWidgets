#include "PageIcons.h"

#include "../DemoHelpers.h"

#include <QApplication>
#include <QClipboard>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QVBoxLayout>

#include "Fluent/FluentCard.h"
#include "Fluent/FluentFlowLayout.h"
#include "Fluent/FluentIcon.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

namespace Demo::Pages {

using namespace Fluent;

namespace {

struct IconSample {
    FluentIconType type;
    const char *enumName;
    QString title;
    QString description;
};

class IconTile final : public QWidget
{
public:
    explicit IconTile(const IconSample &sample, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_sample(sample)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedSize(154, 116);
        setToolTip(QStringLiteral("FluentIconType::%1").arg(QString::fromLatin1(m_sample.enumName)));
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        const auto &tokens = ThemeManager::instance().tokens();
        const auto &colors = ThemeManager::instance().colors();
        const bool hover = m_hover;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF frame = rect().adjusted(0.5, 0.5, -0.5, -0.5);
        QPainterPath panel;
        panel.addRoundedRect(frame, tokens.radius.card, tokens.radius.card);
        const QColor white("#FFFFFF");
        const QColor panelFill = tokens.dark
            ? (hover ? Style::mix(tokens.neutral.card, white, 0.0837)
                     : Style::mix(tokens.neutral.card, white, 0.018))
            : (hover ? tokens.neutral.cardHover : tokens.neutral.card);
        QColor panelBorder = tokens.dark
            ? QColor(255, 255, 255, hover ? 34 : 21)
            : tokens.neutral.strokeSubtle;
        if (hover) {
            panelBorder = tokens.dark ? Style::mix(panelBorder, colors.accent, 0.58) : colors.accent;
            if (tokens.dark) {
                panelBorder.setAlpha(210);
            }
        }
        p.fillPath(panel, panelFill);
        p.setPen(QPen(panelBorder, hover ? 1.2 : 1.0));
        p.drawPath(panel);

        QRectF iconBack(14, 12, 42, 42);
        QPainterPath iconPanel;
        iconPanel.addRoundedRect(iconBack, tokens.radius.control, tokens.radius.control);
        QColor iconPanelColor = tokens.dark
            ? (hover ? Style::mix(tokens.neutral.card, colors.accent, 0.24)
                     : QColor(255, 255, 255, 15))
            : (hover ? tokens.accent.light3 : tokens.neutral.fillTertiary);
        if (!tokens.dark) {
            iconPanelColor.setAlpha(150);
        }
        p.fillPath(iconPanel, iconPanelColor);

        QColor iconColor = hover ? colors.accent : colors.text;
        FluentIconOptions options;
        options.color = iconColor;
        options.autoTheme = false;
        FluentIcon::paintIcon(&p, m_sample.type, iconBack.adjusted(9, 9, -9, -9), options);

        p.setPen(colors.text);
        QFont titleFont = font();
        titleFont.setPointSize(10);
        titleFont.setWeight(QFont::DemiBold);
        p.setFont(titleFont);
        p.drawText(QRectF(66, 14, width() - 78, 20), Qt::AlignLeft | Qt::AlignVCenter, m_sample.title);

        p.setPen(colors.subText);
        QFont enumFont = font();
        enumFont.setPointSize(8);
        p.setFont(enumFont);
        p.drawText(QRectF(66, 34, width() - 78, 20), Qt::AlignLeft | Qt::AlignVCenter, QString::fromLatin1(m_sample.enumName));

        p.setPen(colors.subText);
        QFont descFont = font();
        descFont.setPointSize(8);
        p.setFont(descFont);
        p.drawText(QRectF(14, 68, width() - 28, 36),
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                   m_sample.description);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override
#else
    void enterEvent(QEvent *event) override
#endif
    {
        QWidget::enterEvent(event);
        m_hover = true;
        update();
    }

    void leaveEvent(QEvent *event) override
    {
        QWidget::leaveEvent(event);
        m_hover = false;
        update();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (QClipboard *clipboard = QApplication::clipboard()) {
                clipboard->setText(QStringLiteral("FluentIconType::%1").arg(QString::fromLatin1(m_sample.enumName)));
            }
        }
        QWidget::mousePressEvent(event);
    }

private:
    IconSample m_sample;
    bool m_hover = false;
};

QVector<IconSample> iconSamples()
{
    return {
        {FluentIconType::Home, "Home", DEMO_TEXT("主页", "Home"), DEMO_TEXT("导航主页、仪表板入口", "Home and dashboard entry")},
        {FluentIconType::Menu, "Menu", DEMO_TEXT("菜单", "Menu"), DEMO_TEXT("侧边栏与菜单展开", "Sidebar and menu toggle")},
        {FluentIconType::Back, "Back", DEMO_TEXT("返回", "Back"), DEMO_TEXT("导航返回与历史栈", "Back navigation and history")},
        {FluentIconType::ArrowDown, "ArrowDown", DEMO_TEXT("向下", "Arrow down"), DEMO_TEXT("下载、下移、展开提示", "Download, move down, and reveal hints")},
        {FluentIconType::ArrowUp, "ArrowUp", DEMO_TEXT("向上", "Arrow up"), DEMO_TEXT("上传、上移、返回顶部", "Upload, move up, and return to top")},
        {FluentIconType::ChevronDown, "ChevronDown", DEMO_TEXT("下箭头", "Chevron down"), DEMO_TEXT("折叠区、下拉入口", "Expander and dropdown entry")},
        {FluentIconType::ChevronLeft, "ChevronLeft", DEMO_TEXT("左箭头", "Chevron left"), DEMO_TEXT("轮播、面包屑、返回层级", "Carousel, breadcrumb, and parent level")},
        {FluentIconType::ChevronRight, "ChevronRight", DEMO_TEXT("右箭头", "Chevron right"), DEMO_TEXT("子项展开、进入详情", "Child expansion and detail entry")},
        {FluentIconType::ChevronUp, "ChevronUp", DEMO_TEXT("上箭头", "Chevron up"), DEMO_TEXT("收起、向上折叠", "Collapse and fold upward")},
        {FluentIconType::More, "More", DEMO_TEXT("更多", "More"), DEMO_TEXT("溢出菜单与更多操作", "Overflow menu and more actions")},

        {FluentIconType::Add, "Add", DEMO_TEXT("添加", "Add"), DEMO_TEXT("新增项目、创建动作", "Create and add actions")},
        {FluentIconType::Edit, "Edit", DEMO_TEXT("编辑", "Edit"), DEMO_TEXT("文本、记录、属性编辑", "Edit text, records, or properties")},
        {FluentIconType::Copy, "Copy", DEMO_TEXT("复制", "Copy"), DEMO_TEXT("复制内容或对象", "Copy content or objects")},
        {FluentIconType::Delete, "Delete", DEMO_TEXT("删除", "Delete"), DEMO_TEXT("移除项目、清理内容", "Remove items or clear content")},
        {FluentIconType::Save, "Save", DEMO_TEXT("保存", "Save"), DEMO_TEXT("保存文档或配置", "Save documents or settings")},
        {FluentIconType::Search, "Search", DEMO_TEXT("搜索", "Search"), DEMO_TEXT("查询入口与筛选框", "Search entry and query fields")},
        {FluentIconType::Refresh, "Refresh", DEMO_TEXT("刷新", "Refresh"), DEMO_TEXT("重新加载、同步状态", "Reload and sync state")},
        {FluentIconType::Share, "Share", DEMO_TEXT("分享", "Share"), DEMO_TEXT("共享、发送到外部", "Share or send outward")},

        {FluentIconType::Document, "Document", DEMO_TEXT("文档", "Document"), DEMO_TEXT("文件、表单、页面", "Files, forms, and pages")},
        {FluentIconType::Folder, "Folder", DEMO_TEXT("文件夹", "Folder"), DEMO_TEXT("目录、资源集合", "Folders and resource groups")},
        {FluentIconType::Download, "Download", DEMO_TEXT("下载", "Download"), DEMO_TEXT("导入、保存到本地", "Import or save locally")},
        {FluentIconType::Upload, "Upload", DEMO_TEXT("上传", "Upload"), DEMO_TEXT("导出、发送到云端", "Export or send to cloud")},
        {FluentIconType::Link, "Link", DEMO_TEXT("链接", "Link"), DEMO_TEXT("引用、外链、关系", "References, links, and relations")},
        {FluentIconType::Pin, "Pin", DEMO_TEXT("固定", "Pin"), DEMO_TEXT("置顶、固定到侧栏", "Pin or keep on top")},
        {FluentIconType::Bookmark, "Bookmark", DEMO_TEXT("书签", "Bookmark"), DEMO_TEXT("收藏位置或记录", "Bookmark a place or record")},

        {FluentIconType::Person, "Person", DEMO_TEXT("账户", "Person"), DEMO_TEXT("用户、个人资料、成员", "Users, profiles, and members")},
        {FluentIconType::Lock, "Lock", DEMO_TEXT("安全", "Lock"), DEMO_TEXT("权限、安全、隐私", "Permissions, security, and privacy")},
        {FluentIconType::Mail, "Mail", DEMO_TEXT("邮件", "Mail"), DEMO_TEXT("消息、邮箱、反馈", "Messages, mail, and feedback")},
        {FluentIconType::Bell, "Bell", DEMO_TEXT("通知", "Bell"), DEMO_TEXT("提醒、通知中心", "Alerts and notification center")},
        {FluentIconType::Settings, "Settings", DEMO_TEXT("设置", "Settings"), DEMO_TEXT("配置入口与偏好", "Settings and preferences")},

        {FluentIconType::Info, "Info", DEMO_TEXT("信息", "Info"), DEMO_TEXT("提示与说明", "Information and hints")},
        {FluentIconType::Success, "Success", DEMO_TEXT("成功", "Success"), DEMO_TEXT("完成、通过、成功态", "Completed, passed, or success")},
        {FluentIconType::Warning, "Warning", DEMO_TEXT("警告", "Warning"), DEMO_TEXT("风险、注意、弱错误", "Risk, caution, and warning")},
        {FluentIconType::Checkmark, "Checkmark", DEMO_TEXT("勾选", "Checkmark"), DEMO_TEXT("选择、确认、完成", "Select, confirm, and complete")},
        {FluentIconType::Close, "Close", DEMO_TEXT("关闭", "Close"), DEMO_TEXT("关闭、取消、移除", "Close, cancel, and dismiss")},
        {FluentIconType::Star, "Star", DEMO_TEXT("星标", "Star"), DEMO_TEXT("重点、评分、收藏", "Featured, rating, and favorite")},
        {FluentIconType::Heart, "Heart", DEMO_TEXT("喜欢", "Heart"), DEMO_TEXT("喜欢、关注、偏好", "Like, follow, and preference")},

        {FluentIconType::Input, "Input", DEMO_TEXT("输入", "Input"), DEMO_TEXT("输入框与文本控件", "Inputs and text controls")},
        {FluentIconType::Controls, "Controls", DEMO_TEXT("控件", "Controls"), DEMO_TEXT("滑杆、开关、调节", "Sliders, toggles, and controls")},
        {FluentIconType::Calendar, "Calendar", DEMO_TEXT("日期", "Calendar"), DEMO_TEXT("日期、日程、时间", "Date, schedule, and time")},
        {FluentIconType::Gauge, "Gauge", DEMO_TEXT("仪表", "Gauge"), DEMO_TEXT("角度、进度、指标", "Angle, progress, and metrics")},
        {FluentIconType::Data, "Data", DEMO_TEXT("数据", "Data"), DEMO_TEXT("表格、列表、数据视图", "Tables, lists, and data views")},
        {FluentIconType::Layout, "Layout", DEMO_TEXT("布局", "Layout"), DEMO_TEXT("容器、页面结构", "Containers and page structure")},
        {FluentIconType::Dialog, "Dialog", DEMO_TEXT("对话", "Dialog"), DEMO_TEXT("弹窗、提示、引导", "Dialogs, popups, and tips")},
        {FluentIconType::Window, "Window", DEMO_TEXT("窗口", "Window"), DEMO_TEXT("窗口 chrome 与桌面面板", "Window chrome and desktop panels")},
        {FluentIconType::Play, "Play", DEMO_TEXT("播放", "Play"), DEMO_TEXT("动态、媒体、开始", "Motion, media, and start")},
        {FluentIconType::Icons, "Icons", DEMO_TEXT("图标", "Icons"), DEMO_TEXT("图标集合与资源页", "Icon gallery and resources")},
        {FluentIconType::Eye, "Eye", DEMO_TEXT("查看", "Eye"), DEMO_TEXT("预览、显示、可见性", "Preview, show, and visibility")},
        {FluentIconType::Filter, "Filter", DEMO_TEXT("筛选", "Filter"), DEMO_TEXT("条件过滤、结果收窄", "Filter and narrow results")},
        {FluentIconType::Sort, "Sort", DEMO_TEXT("排序", "Sort"), DEMO_TEXT("列表排序和组织", "Sort and organize lists")},
        {FluentIconType::ZoomIn, "ZoomIn", DEMO_TEXT("放大", "Zoom in"), DEMO_TEXT("画布、图像、预览缩放", "Canvas, image, and preview zoom")},
        {FluentIconType::ZoomOut, "ZoomOut", DEMO_TEXT("缩小", "Zoom out"), DEMO_TEXT("缩小视图或预览", "Zoom out views or previews")},
    };
}

} // namespace

QWidget *createIconsPage()
{
    return Demo::makePage([](QVBoxLayout *page) {
        auto s = Demo::makeSection(
            DEMO_TEXT("图标", "Icons"),
            DEMO_TEXT("内置 FluentIcon 图标集：统一 24px outline 风格，可直接用于 NavigationView、按钮、菜单、状态提示和命令栏。",
                      "Built-in FluentIcon set: a consistent 24px outline style for NavigationView, buttons, menus, status messages, and command bars."));
        page->addWidget(s.card);

        auto *hint = new FluentLabel(DEMO_TEXT(
            "点击任意图标可复制对应的 FluentIconType 枚举。当前图标使用 QPainter 原生绘制，颜色会跟随主题与控件状态，不依赖外部字体文件。",
            "Click any icon to copy its FluentIconType enum. Icons are painted natively with QPainter, follow theme/control state colors, and do not depend on external icon fonts."));
        hint->setWordWrap(true);
        hint->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.88;"));
        s.body->addWidget(hint);

        auto *paletteCard = new FluentCard();
        auto *paletteLayout = new QVBoxLayout(paletteCard);
        paletteLayout->setContentsMargins(18, 16, 18, 18);
        paletteLayout->setSpacing(12);

        auto *title = new FluentLabel(DEMO_TEXT("图标矩阵", "Icon matrix"));
        title->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 650;"));
        paletteLayout->addWidget(title);

        auto *flowHost = new QWidget(paletteCard);
        auto *flow = new FluentFlowLayout(flowHost, 0, 10, 10);
        flow->setUniformItemWidthEnabled(true);
        flow->setMinimumItemWidth(154);
        flow->setColumnHysteresis(12);
        flow->setAnimationEnabled(true);
        flow->setAnimationDuration(120);
        flowHost->setLayout(flow);

        const QVector<IconSample> samples = iconSamples();
        for (const IconSample &sample : samples) {
            flow->addWidget(new IconTile(sample, flowHost));
        }

        paletteLayout->addWidget(flowHost);
        page->addWidget(paletteCard);

        auto *usageCard = Demo::makeCollapsedCard(
            DEMO_TEXT("使用方式", "Usage"),
            DEMO_TEXT("FluentIcon 可以生成 QIcon，也可以直接绘制到任意 QPainter。",
                      "FluentIcon can create QIcon objects or paint directly into any QPainter."),
            DEMO_TEXT("要点：\n"
                      "-FluentIcon::icon(type) 适合 QAction/QAbstractButton\n"
                      "-FluentIcon::paintIcon() 适合自绘控件\n"
                      "-FluentIconOptions 可控制颜色、透明度以及反色模式",
                      "Highlights:\n"
                      "-FluentIcon::icon(type) fits QAction/QAbstractButton\n"
                      "-FluentIcon::paintIcon() fits custom-painted widgets\n"
                      "-FluentIconOptions controls color, opacity, and reversed mode"),
            QStringLiteral("auto icon = Fluent::FluentIcon::icon(Fluent::FluentIconType::Search);\n"
                           "action->setIcon(icon);\n\n"
                           "Fluent::FluentIconOptions options;\n"
                           "options.color = palette().color(QPalette::WindowText);\n"
                           "Fluent::FluentIcon::paintIcon(&painter,\n"
                           "                              Fluent::FluentIconType::Settings,\n"
                           "                              iconRect,\n"
                           "                              options);"),
            false);
        page->addWidget(usageCard);
    });
}

} // namespace Demo::Pages
