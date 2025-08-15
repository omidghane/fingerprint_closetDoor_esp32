from django.contrib import admin
from django.conf.urls.static import static
from django.urls import path, include

from src import settings

urlpatterns = ([
    path('admin2/', admin.site.urls),
    # path('', include('home.urls', namespace='home')),
    path('accounts/', include('accounts.urls', namespace='accounts')),
    # path('task/', include('task.urls', namespace='task')),
    path('configboard/', include('ConfigBoards.urls', namespace='Config')),
] + static(settings.STATIC_URL, document_root=settings.STATIC_ROOT) +
static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT))
