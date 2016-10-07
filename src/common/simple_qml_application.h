/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SIMPLE_QML_APPLICATION_H_
# define MEDIAN_PATH_SIMPLE_QML_APPLICATION_H_
# include "../../median-path/median_path.h"
# include <graphics-origin/application/renderer.h>
# include <QtQuick/QQuickView>

namespace median_path {
  class simple_qml_application
    : public QQuickView {
    Q_OBJECT
  public:
    explicit simple_qml_application( QWindow* parent = nullptr );
    ~simple_qml_application();
  };
}
# endif
